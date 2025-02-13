#include "MachineComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameInput/Input.h>

#include "MachinePartComponent.h"
#include "../../Application.h"

#include "../Editor/EditorComponent.h" // 为了 ESC 键返回编辑器的临时测试用途

REGISTER_ENTITY_COMPONENT(CMachineComponent);

std::unordered_set<CMachineComponent *> CMachineComponent::s_workingMachines {};
std::mutex CMachineComponent::s_workingMachinesMutex {};

Frame::EntityEvent::Flags CMachineComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update | Frame::EntityEvent::EFlag::Render;
}

void CMachineComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
	{
		// 临时测试用的
		if(Frame::gInput->pKeyboard->GetPressed(Frame::EKeyId::eKI_Escape)) {
			for(auto & [_,pEntity] : Frame::gEntitySystem->GetEntities()) {
				if(auto pEditor = pEntity->GetComponent<CEditorComponent>()) {
					pEditor->SetWorking(true);
					break;
				}
			}
			RemoveEntityAtTheEndOfThisFrame(m_pEntity->GetId());
		}

		std::lock_guard<std::mutex> lock { m_stepMutex };

		const int xRelative = -static_cast<int>(Frame::gInput->pKeyboard->GetHolding(Frame::EKeyId::eKI_A)) + static_cast<int>(Frame::gInput->pKeyboard->GetHolding(Frame::EKeyId::eKI_D));
		const int yRelative = -static_cast<int>(Frame::gInput->pKeyboard->GetHolding(Frame::EKeyId::eKI_W)) + static_cast<int>(Frame::gInput->pKeyboard->GetHolding(Frame::EKeyId::eKI_S));
		if(xRelative == 0 && yRelative == 0) {
			m_targetMovingDir = 0.f;
		} else {
			m_targetMovingDir = Frame::Vec2 { static_cast<float>(xRelative), static_cast<float>(yRelative) };
			m_targetMovingDir.Rotate(Frame::gCamera->GetRotation());
			m_targetMovingDir.Normalize();
		}

		float rot = Frame::gCamera->GetViewRotation();
		GUIBegin();
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(100.f - 70.f, 100.f + 70.f, 0x000000, .5f);
		Frame::gRenderer->pShapeRenderer->DrawLineBlended(100.f, m_targetMovingDir * 70.f + 100.f, 0xFFFFFF, 1.f, 2.f);
		Frame::gRenderer->pShapeRenderer->DrawLineBlended(100.f, m_targetMovingDir.GetRotated(rot) * 70.f + 100.f, 0xFFFFFF, .5f, 2.f);
		GUIEnd();
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
	{
		std::lock_guard<std::mutex> lock { m_stepMutex };
		
		for(auto & pMachinePart : m_machineParts) {
			pMachinePart->Render();
		}
	}
	break;
	}
}

void CMachineComponent::OnShutDown() {
	std::lock_guard<std::mutex> machinesLock { s_workingMachinesMutex };
	
	if(auto it = s_workingMachines.find(this); it != s_workingMachines.end()) {
		s_workingMachines.erase(it);
	}

	for(auto & pMachinePart : m_machineParts) {
		Frame::gEntitySystem->RemoveEntity(pMachinePart->GetEntity()->GetId());
	}
}

void CMachineComponent::Step(float timeStep) {

	std::lock_guard<std::mutex> lock { m_stepMutex };
	
	for(auto & pMachinePart : m_machineParts) {
		if(!pMachinePart->isMainPart) {
			continue;
		}
		pMachinePart->Step(timeStep, m_targetMovingDir);
	}
}

nlohmann::json CMachineComponent::Serialize() const {
	return {}; // TODO
}

// TODO - 跨机器部分的关节类装置的处理
void CMachineComponent::Deserialize(const nlohmann::json & json) {
	
	std::lock_guard<std::mutex> lock { m_stepMutex };

	std::vector<SSerializedMachinePart> machineParts;

	Frame::Vec2 cabinPos;
	float cabinRot = 0.f;

	// 解析 JSON
	try {
		for(const auto & machinePartJson : json) {
			SSerializedMachinePart machinePart;

			for(const auto & deviceJson : machinePartJson) {
				SSerializedDevice device = SSerializedDevice::MakeFromJson(deviceJson);
				if(device.guid == GetDeviceConfig<SCabinDeviceData>().guid) {
					cabinPos = device.position;
					cabinRot = device.rotation;
				}
				machinePart.devices.push_back(device);
			}

			machineParts.push_back(machinePart);
		}
	} catch(const nlohmann::json::exception & e) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "CMachineComponent::Deserialize(): Illegal JSON: %s", e.what());
		return;
	}

	// 召唤！苏醒吧，机器！！！
	for(const auto & serializedMachinePart : machineParts) {
		Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity();
		CMachinePartComponent * pMachinePart = pEntity->CreateComponent<CMachinePartComponent>();

		for(const auto & serializedDevice : serializedMachinePart.devices) {
			auto pDevice = GetDeviceData(serializedDevice.guid)->NewShared();
			pDevice->SetRelativePositionRotation(serializedDevice.position - cabinPos, serializedDevice.rotation - cabinRot);
			pMachinePart->devices.insert(pDevice);
		}
		pMachinePart->Initialize(pMachinePart->devices, m_colorSet);

		m_machineParts.insert(pMachinePart);
	}
}