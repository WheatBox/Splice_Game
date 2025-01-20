#include "PhysicsWorldComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>

#include "../Application.h"
#include "../Depths.h"
#include "../Utility.h"

#include "SmokeEmitterComponent.h"
#include "Machine/MachineComponent.h"

#include <thread>

REGISTER_ENTITY_COMPONENT(CPhysicsWorldComponent);

CPhysicsWorldComponent * CPhysicsWorldComponent::s_pPhysicsWorldComponent = nullptr;

std::thread * CPhysicsWorldComponent::s_pPhysicsThread = nullptr;

std::queue<std::function<void ()>> CPhysicsWorldComponent::s_physicalizeQueue {};

void CPhysicsWorldComponent::Initialize() {
	m_pEntity->SetZDepth(Depths::PhysicsWorld);

	if(!s_pPhysicsThread) {
		s_pPhysicsThread = new std::thread {
		[]() {
			static std::chrono::steady_clock::time_point beforeLoopTimePoint;

			constexpr double hz = 60.0;
			constexpr float timeStep = 1.f / static_cast<float>(hz);
			std::chrono::duration<double> target(1.0 / hz);

			std::chrono::duration<double> frameTime(0.0);
			std::chrono::duration<double> sleepAdjust(0.0);

			while(true) {
				std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

				if(s_pPhysicsWorldComponent) {
					s_pPhysicsWorldComponent->Step(timeStep);
				}

				{
					// https://github.com/erincatto/box2d/blob/main/testbed/main.cpp

					// Throttle to cap at 60Hz. This adaptive using a sleep adjustment. This could be improved by
					// using mm_pause or equivalent for the last millisecond.
					std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
					std::chrono::duration<double> timeUsed = t2 - t1;
					std::chrono::duration<double> sleepTime = target - timeUsed + sleepAdjust;
					if (sleepTime > std::chrono::duration<double>(0))
					{
						std::this_thread::sleep_for(sleepTime);
					}

					std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
					frameTime = t3 - t1;

					// Compute the sleep adjustment using a low pass filter
					sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
				}

				//printf("physics fps: %f\n", 1.f / (float)frameTime.count());
			}
		}
		};
		s_pPhysicsThread->detach();
	}

	if(b2World_IsValid(gWorldId)) {
		b2WorldId worldIdTemp = gWorldId;
		gWorldId = b2_nullWorldId;
		b2DestroyWorld(worldIdTemp);
		// 这里分成这三行写主要是防止多线程的访问出错
	}
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.f, 0.f };
		gWorldId = b2CreateWorld(& worldDef);
	}

	s_pPhysicsWorldComponent = this;

	m_pCameraComponent = m_pEntity->CreateComponent<CCameraComponent>();
	if(m_pCameraComponent) {
		m_pCameraComponent->Initialize(
			[]() { return Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Middle); },
			[]() { return Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Right); }
		);
		m_pCameraComponent->SetWorking(false);
	}

	m_pSmokeEmitterEntity = Frame::gEntitySystem->SpawnEntity();
	if(m_pSmokeEmitterEntity) {
		m_pSmokeEmitterEntity->CreateComponent<CSmokeEmitterComponent>();
	}
}
void CPhysicsWorldComponent::OnShutDown() {
	if(s_pPhysicsWorldComponent == this) {
		s_pPhysicsWorldComponent = nullptr;

		if(b2World_IsValid(gWorldId)) {
			b2WorldId worldIdTemp = gWorldId;
			gWorldId = b2_nullWorldId;
			b2DestroyWorld(worldIdTemp);
		}
	}

	if(m_pSmokeEmitterEntity) {
		Frame::gEntitySystem->RemoveEntity(m_pSmokeEmitterEntity->GetId());
	}
}

void CPhysicsWorldComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	if(s_pPhysicsWorldComponent != this) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::BeforeUpdate:
	{
		//printf("fps: %f\n", 1.f / event.params[0].f);
	}
	break;
	case Frame::EntityEvent::Render:
		if(m_bEditorWorking) {
			break;
		}
		if(m_pCameraComponent) {
			m_pCameraComponent->CameraControl(true);
		}
		DrawBlockBackground();
		break;
	}
}

void CPhysicsWorldComponent::Step(float timeStep) {
	if(!b2World_IsValid(gWorldId)) {
		return;
	}

	while(!s_physicalizeQueue.empty()) {
		s_physicalizeQueue.front()();
		s_physicalizeQueue.pop();
	}

	std::lock_guard<std::mutex> machinesLock { CMachineComponent::s_workingMachinesMutex };

	for(auto & pMachine : CMachineComponent::s_workingMachines) {
		pMachine->Step(timeStep);
	}

	b2World_Step(gWorldId, timeStep, 4);
	// 因为参数 timeStep 应为不能变的固定参数，所以为物理计算单独开一条线程，其它的物理相关运算应该也放入该线程
	// TODO - 以及记得给后两个参数都做成玩家开房间时候的设置选项

}
