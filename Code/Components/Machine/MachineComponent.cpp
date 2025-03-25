#include "MachineComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameInput/Input.h>

#include "MachinePartComponent.h"
#include "../../Application.h"
#include "../../Devices/Devices.h"
#include "../PhysicsWorldComponent.h"

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
		
		Frame::gCamera->PushOntoStack();
		for(auto & pMachinePart : m_machineParts) {
			pMachinePart->RenderReady();
		}
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_Connector); }
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_StaticBottom); }
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_Static); }
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_Dynamic); }
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_StaticTop); }
		for(auto & pMachinePart : m_machineParts) { pMachinePart->Render(eDSG_DynamicTop); }
		Frame::gCamera->PopFromStack();
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
		Frame::Vec2 targetMovingDir {};
		if(pMachinePart->isMainPart) {
			targetMovingDir = m_targetMovingDir;
		}
		pMachinePart->Step(timeStep, targetMovingDir);
	}
}

nlohmann::json CMachineComponent::Serialize() const {
	return {}; // TODO
}

void __RecurMachinePart(std::shared_ptr<IDeviceData> pDevice, std::unordered_set<std::shared_ptr<IDeviceData>> & devices, std::unordered_set<std::shared_ptr<IDeviceData>> & ignores) {
	if(!pDevice || ignores.find(pDevice) != ignores.end()) {
		return;
	}
	devices.insert(pDevice);
	ignores.insert(pDevice);
	for(auto & [_, interface] : pDevice->m_interfaces) {
		if(interface.to) {
			__RecurMachinePart(interface.to->from.lock(), devices, ignores);
		}
	}
}

void CMachineComponent::Deserialize(const nlohmann::json & json) {

	printf("\n%s\n", json.dump().c_str());
	
	std::lock_guard<std::mutex> lock { m_stepMutex };

	SSerializedMachine serializedMachine;
	// 解析 JSON
	try {
		serializedMachine.FromJson(json);
	} catch(const nlohmann::json::exception & e) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "CMachineComponent::Deserialize(): Illegal JSON: %s", e.what());
		return;
	}

	Frame::Vec2 cabinPos;
	float cabinRot = 0.f;
	for(const auto & serializedDevice : serializedMachine.devices) {
		if(serializedDevice.guid == GetDeviceConfig<SCabinDeviceData>().guid) {
			cabinPos = serializedDevice.position;
			cabinRot = serializedDevice.rotation;
		}
	}

	std::shared_ptr<IDeviceData> pCabin = nullptr;

	std::vector<std::shared_ptr<IDeviceData>> devices; 
	// 从 json 中解析得到的信息转换为实际的装置数据
	for(const auto & serializedDevice : serializedMachine.devices) {

		std::shared_ptr<IDeviceData> pDevice = GetDeviceData(serializedDevice.guid)->NewShared();
		pDevice->SetRelativePositionRotation(serializedDevice.position - cabinPos, serializedDevice.rotation - cabinRot);
		for(const auto & [interfaceID, interfaceDef] : pDevice->GetInterfaceDefs()) {
			pDevice->m_interfaces.insert({ interfaceID, { pDevice, interfaceID, interfaceDef }});
		}
		for(const auto & connection : serializedDevice.connections) {
			IDeviceData::ConnectInterfaces(& pDevice->m_interfaces.at(connection.myInterfaceID), & devices[connection.to]->m_interfaces.at(connection.toInterfaceID));
		}

		if(!pCabin && pDevice->GetConfig().guid == GetDeviceConfig<SCabinDeviceData>().guid) {
			pCabin = pDevice;
		}

		devices.push_back(pDevice);

	}

	if(!pCabin) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "CMachineComponent::Deserialize(): Can not find Cabin!");
		return;
	}

	std::unordered_set<std::shared_ptr<IDeviceData>> ignores;

	for(auto & pDevice : devices) {
		std::unordered_set<std::shared_ptr<IDeviceData>> machinePart;

		__RecurMachinePart(pDevice, machinePart, ignores);

		if(machinePart.empty()) {
			continue;
		}

		Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity();
		CMachinePartComponent * pMachinePartComp = pEntity->CreateComponent<CMachinePartComponent>();
		pMachinePartComp->devices = machinePart;
		pMachinePartComp->Initialize(pMachinePartComp->devices, m_colorSet);

		m_machineParts.insert(pMachinePartComp);
	}

	// 处理所有关节装置，为它们创建物理关节
	for(const auto & serializedJointRelation : serializedMachine.jointRelations) {
		std::vector<std::shared_ptr<IDeviceData>> devicesOfCurrentJoint;
		for(size_t i = 0, len = serializedJointRelation.deviceIndices.size(); i < len; i++) {
			devicesOfCurrentJoint.push_back(devices[serializedJointRelation.deviceIndices[i]]);
		}
		for(size_t i = 0, len = serializedJointRelation.deviceIndices.size(); i < len; i++) {
			std::shared_ptr<IDeviceData> pRootDevice = devices[serializedJointRelation.deviceIndices[i]];
			if(pRootDevice->GetConfig().isJointRoot) {
				CPhysicsWorldComponent::s_physicalizeQueue.push(
					[pRootDevice, devicesOfCurrentJoint]() {
						pRootDevice->InitJoint(devicesOfCurrentJoint);
					}
				);
			}
		}
	}
}