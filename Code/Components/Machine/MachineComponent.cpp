#include "MachineComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameInput/Input.h>

#include "MachinePartComponent.h"
#include "DeviceComponent.h"
#include "../Editor/EditorDeviceComponent.h"
#include "../../Application.h"

REGISTER_ENTITY_COMPONENT(CMachineComponent);

std::unordered_set<CMachineComponent *> CMachineComponent::s_workingMachines {};
std::mutex CMachineComponent::s_workingMachinesMutex {};

Frame::EntityEvent::Flags CMachineComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update;
}

void CMachineComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Update:
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
	}
}

static bool __HasConnected(const std::unordered_map<CEditorDeviceComponent *, std::unordered_set<CEditorDeviceComponent *>> & connectedEDComps, CEditorDeviceComponent * pEDComp1, CEditorDeviceComponent * pEDComp2) {
	if(!pEDComp1 || !pEDComp2) {
		return false;
	}

	for(auto pEDComp = pEDComp1, pEDCompAnother = pEDComp2; pEDComp != nullptr; pEDCompAnother = pEDComp1, (pEDComp = (pEDComp == pEDComp1) ? pEDComp2 : nullptr)) {
		auto it = connectedEDComps.find(pEDComp);
		if(it == connectedEDComps.end()) {
			continue;
		}
		if(it->second.find(pEDCompAnother) != it->second.end()) {
			return true;
		}
	}

	return false;
};

static void __Connect(std::unordered_map<CEditorDeviceComponent *, std::unordered_set<CEditorDeviceComponent *>> & connectedEDComps, const std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> & map_EDCompDeviceComp, CEditorDeviceComponent * pEDComp1, CEditorDeviceComponent * pEDComp2) {
	if(!pEDComp1 || !pEDComp2) {
		return;
	}

	for(auto pEDComp = pEDComp1, pEDCompAnother = pEDComp2; pEDComp != nullptr; pEDCompAnother = pEDComp1, (pEDComp = (pEDComp == pEDComp1) ? pEDComp2 : nullptr)) {
		if(auto it = connectedEDComps.find(pEDComp); it == connectedEDComps.end()) {
			connectedEDComps.insert({ pEDComp, { pEDCompAnother, nullptr } });
		} else {
			it->second.insert(pEDCompAnother);
		}
	}

	for(auto pEDComp = pEDComp1; pEDComp != nullptr; pEDComp = (pEDComp == pEDComp1) ? pEDComp2 : nullptr) {
		CEditorDeviceComponent * pJointEDComp = pEDComp;
		CDeviceComponent * pJointDevice = nullptr;
		if(auto it = map_EDCompDeviceComp.find(pJointEDComp); it != map_EDCompDeviceComp.end()) {
			pJointDevice = it->second;
		} else {
			continue;
		}

		switch(pJointDevice->GetDeviceType()) {
		case IDeviceData::EType::Joint:
		{
			auto pData = reinterpret_cast<SJointDeviceData *>(pJointDevice->GetNode()->pDeviceData);

			if(!pData) {
				break;
			}

			int dirIndexToAnotherEDComp = GetMachinePartJointDevicePointDirIndex(pJointEDComp);

			auto pAnotherEDComp = pJointEDComp->m_neighbors[dirIndexToAnotherEDComp];
			auto pBehindEDComp = pJointEDComp->m_neighbors[GetRevDirIndex(pJointEDComp->GetDirIndex())];
			
			auto itAnotherEDComp = map_EDCompDeviceComp.find(pAnotherEDComp);
			auto itBehindEDComp = map_EDCompDeviceComp.find(pBehindEDComp);
			Frame::Vec2 myPos = pJointDevice->GetEntity()->GetPosition();
			Frame::Vec2 anotherPos = myPos;
			if(itAnotherEDComp != map_EDCompDeviceComp.end()) {
				anotherPos = itAnotherEDComp->second->GetEntity()->GetPosition();
			}
			pData->Initialize(
				itAnotherEDComp == map_EDCompDeviceComp.end() ? nullptr : itAnotherEDComp->second,
				itBehindEDComp == map_EDCompDeviceComp.end() ? nullptr : itBehindEDComp->second,
				anotherPos != myPos ? 0.f : ((anotherPos - myPos).Degree() - pJointDevice->GetEntity()->GetRotation()),
				dirIndexToAnotherEDComp
			);
		}
		break;
		}
	}
}

static bool __ConnectMachinePartsByDevices(
	const std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> & map_EDCompDeviceComp,
	CEditorDeviceComponent * pEDComp1,
	CEditorDeviceComponent * pEDComp2
) {
	CDeviceComponent * pDevice1 = nullptr, * pDevice2 = nullptr;
	if(auto it = map_EDCompDeviceComp.find(pEDComp1); it != map_EDCompDeviceComp.end()) {
		pDevice1 = it->second;
	}
	if(auto it = map_EDCompDeviceComp.find(pEDComp2); it != map_EDCompDeviceComp.end()) {
		pDevice2 = it->second;
	}
	if(!pDevice1 || !pDevice2) {
		return false;
	}

	CMachinePartComponent * pBasicMachinePart = nullptr;
	if(auto pEnt = pDevice1->GetMachinePartEntity()) {
		pBasicMachinePart = pEnt->GetComponent<CMachinePartComponent>();
	}
	if(!pBasicMachinePart) {
		return false;
	}

	CMachinePartComponent * pAnotherMachinePart = nullptr;
	if(auto pEnt = pDevice2->GetMachinePartEntity()) {
		pAnotherMachinePart = pEnt->GetComponent<CMachinePartComponent>();
	}
	if(!pAnotherMachinePart) {
		return false;
	}

	return pBasicMachinePart->CreateJointWith(pAnotherMachinePart, pDevice1);
}

void CMachineComponent::Initialize(CEditorDeviceComponent * pDeviceCabin, const SColorSet & colorSet) {

	std::lock_guard<std::mutex> machinesLock { s_workingMachinesMutex };
	s_workingMachines.insert(this);

	std::unordered_set<CEditorDeviceComponent *> ignoreDevices; // 已经建立的装置
	std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> map_EDCompDeviceComp;

	// 递归创建各个机器部分
	std::function<void (CEditorDeviceComponent *)> recursive = [&](CEditorDeviceComponent * pEDComp) {

		if(!pEDComp) {
			return;
		}

		if(ignoreDevices.find(pEDComp) != ignoreDevices.end()) {
			return;
		}

		std::unordered_set<CEditorDeviceComponent *> currentPartDevices;
		std::unordered_set<CEditorDeviceComponent *> jointDevices;

		RecursiveMachinePartEditorDevices(& currentPartDevices, & jointDevices, pEDComp, ignoreDevices);

		for(auto & pJointEDComp : jointDevices) {
			if(auto pBack = pJointEDComp->m_neighbors[GetRevDirIndex(pJointEDComp->GetDirIndex())]) {
				if(currentPartDevices.find(pBack) != currentPartDevices.end()) {
					currentPartDevices.insert(pJointEDComp);
				}
			} else {
				currentPartDevices.insert(pJointEDComp);
			}
		}

		if(currentPartDevices.empty()) {
			return;
		}

		if(auto pEnt = Frame::gEntitySystem->SpawnEntity()) {
			if(auto pComp = pEnt->CreateComponent<CMachinePartComponent>()) {
				m_machineParts.insert(pComp);

				std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> mapTemp;
				pComp->Initialize(& mapTemp, this, currentPartDevices, colorSet);
				map_EDCompDeviceComp.insert(mapTemp.begin(), mapTemp.end());
			}
		}

		ignoreDevices.insert(currentPartDevices.begin(), currentPartDevices.end());

		if(jointDevices.empty()) {
			return;
		}

		for(auto & pJointEDComp : jointDevices) {
			recursive(pJointEDComp->m_neighbors[GetRevDirIndex(pJointEDComp->GetDirIndex())]);
			recursive(pJointEDComp->m_neighbors[GetMachinePartJointDevicePointDirIndex(pJointEDComp)]);
		}
	};
	recursive(pDeviceCabin);

	std::unordered_map<CEditorDeviceComponent *, std::unordered_set<CEditorDeviceComponent *>> connectedEDComps;
	
	for(auto & pJointEDComp : ignoreDevices) {
		if(!pJointEDComp || !IsMachinePartJoint(pJointEDComp->GetDeviceType())) {
			continue;
		}

		/*
		首先检查前方的装置是否已经与自己建立过连接
			若否，与之建立连接
		检查后方的装置是否已经与自己建立过连接
			若否，检查该装置与自己是否在同一个机器部分 // 好像没必要，因为在 __ConnectMachinePartsByDevices() 要调用的 CMachinePartComponent::CreateJointWith() 中有相关验证
				若否，与之建立连接
		*/

		do {
			CEditorDeviceComponent * pEDComp = pJointEDComp->m_neighbors[GetMachinePartJointDevicePointDirIndex(pJointEDComp)];
			if(!pEDComp) {
				break;
			}
			if(__HasConnected(connectedEDComps, pJointEDComp, pEDComp)) {
				break;
			}

			if(__ConnectMachinePartsByDevices(map_EDCompDeviceComp, pJointEDComp, pEDComp) || 1) {
				__Connect(connectedEDComps, map_EDCompDeviceComp, pJointEDComp, pEDComp);
			}
		} while(false);

		do {
			CEditorDeviceComponent * pEDComp = pJointEDComp->m_neighbors[GetRevDirIndex(pJointEDComp->GetDirIndex())];
			if(!pEDComp) {
				break;
			}
			if(__HasConnected(connectedEDComps, pJointEDComp, pEDComp)) {
				break;
			}

			if(__ConnectMachinePartsByDevices(map_EDCompDeviceComp, pEDComp, pJointEDComp) || 1) {
				__Connect(connectedEDComps, map_EDCompDeviceComp, pEDComp, pJointEDComp);
			}
		} while(false);
	}
}

void CMachineComponent::OnShutDown() {
	std::lock_guard<std::mutex> machinesLock { s_workingMachinesMutex };
	
	if(auto it = s_workingMachines.find(this); it != s_workingMachines.end()) {
		s_workingMachines.erase(it);
	}

	for(auto & pMachinePartComp : m_machineParts) {
		Frame::gEntitySystem->RemoveEntity(pMachinePartComp->GetEntity()->GetId());
	}
}

void CMachineComponent::Step(float timeStep) {

	std::lock_guard<std::mutex> lock { m_stepMutex };

	for(auto & pPart : m_machineParts) {
		if(!pPart->IsMainPart()) {
			//continue;
		}
		pPart->SetTargetMovingDir(m_targetMovingDir);
	}

	for(auto & pPart : m_machineParts) {
		pPart->Step(timeStep);
	}
}
