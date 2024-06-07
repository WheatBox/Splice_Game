#include "PhysicsWorldComponent.h"

#include <FrameEntity/Entity.h>

#include "../Application.h"
#include "../Depths.h"
#include "../Utility.h"

REGISTER_ENTITY_COMPONENT(, CPhysicsWorldComponent);

CPhysicsWorldComponent * CPhysicsWorldComponent::s_pPhysicsWorldComponent = nullptr;

void CPhysicsWorldComponent::Initialize() {
	m_pEntity->SetZDepth(Depths::PhysicsWorld);

	if(gWorld) {
		delete gWorld;
	}
	gWorld = new b2World { { 0.f, 10.f } };
	gWorld->SetGravity({ 0.f, 0.f });
	gWorld->SetAutoClearForces(true);
	gWorld->SetAllowSleeping(true);

	s_pPhysicsWorldComponent = this;

	m_pCameraComponent = m_pEntity->CreateComponent<CCameraComponent>();
	if(m_pCameraComponent) {
		m_pCameraComponent->Initialize(
			[]() { return Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Middle); },
			[]() { return Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Right); }
		);
		m_pCameraComponent->SetWorking(false);
	}
}
void CPhysicsWorldComponent::OnShutDown() {
	if(!gWorld)
		return;
	delete gWorld;
	gWorld = nullptr;

	if(s_pPhysicsWorldComponent == this) {
		s_pPhysicsWorldComponent = nullptr;
	}
}

void CPhysicsWorldComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	if(s_pPhysicsWorldComponent != this) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::BeforeUpdate:
	{
		//gWorld->Step(event.params[0].f, 8, 3);
		//gWorld->Step(event.params[0].f, 16, 8);
		gWorld->Step(1.f / 60.f, 8, 3);
		// TODO - 因为第一个参数应为不能变的固定参数，所以应该为这一步单独开一条线程
		// 以及记得给这三个参数都做成玩家开房间时候的设置选项
	}
	break;
	case Frame::EntityEvent::Render:
		if(m_bEditorWorking) {
			break;
		}
		if(m_pCameraComponent) {
			m_pCameraComponent->CameraControl();
		}
		DrawBlockBackground();
		break;
	}
}
