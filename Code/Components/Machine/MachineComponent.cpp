#include "MachineComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>

#include "DeviceComponent.h"
#include "../Editor/EditorDeviceComponent.h"
#include "DeviceConnectorRendererComponent.h"
#include "../RigidbodyComponent.h"

#include "../../Depths.h"
#include "../../Pipe.h"

REGISTER_ENTITY_COMPONENT(, CMachineComponent);

Frame::EntityEvent::Flags CMachineComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::Render;
}

void CMachineComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Update:
		if(m_pRigidbodyComponent) {
			m_pEntity->SetPosition(m_pRigidbodyComponent->GetPosition());
			m_pEntity->SetRotation(m_pRigidbodyComponent->GetRotation());
		}
		break;
	case Frame::EntityEvent::Render:
		for(const auto & pipe : m_pipes) {
			DrawPipe<SPipeNode>(pipe, m_pEntity->GetPosition(), m_colorSet.pipe, 1.f, m_pEntity->GetRotation());
		}
		break;
	}
}

void CMachineComponent::Initialize(const std::unordered_set<CEditorDeviceComponent *> & editorDeviceComps, const std::vector<std::vector<SEditorPipeNode *>> & pipes, const SColorSet & colorSet) {

	m_pEntity->SetZDepth(Depths::Machine);

	m_colorSet = colorSet;

	m_pRigidbodyComponent = m_pEntity->CreateComponent<CRigidbodyComponent>();
	std::vector<b2FixtureDef *> fixtureDefs;
	std::unordered_map<b2FixtureDef *, CDeviceComponent *> map_fixtureDefDeviceComp;

	/* --------------------- 创建装置 --------------------- */

	std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> map_EDCompDeviceComp;

	for(const auto & pEDComp : editorDeviceComps) {
		if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
			if(CDeviceComponent * pComp = pEntity->CreateComponent<CDeviceComponent>()) {
				Frame::CEntity * pEDCompEnt = pEDComp->GetEntity();
				const Frame::Vec2 devicePos = pEDCompEnt->GetPosition();
				const float deviceRot = pEDCompEnt->GetRotation();
				pEntity->SetPosition(devicePos);
				pEntity->SetRotation(deviceRot);

				pComp->Initialize(m_pEntity, pEDComp->GetDeviceType(), pEDComp->GetKeyId(), pEDComp->GetDirIndex(), colorSet);
				pComp->SetRelativePositionRotation(devicePos, deviceRot);

				auto defs = CDeviceComponent::CreateFixtureDefs(pEDComp->GetDeviceType(), devicePos, deviceRot);
				fixtureDefs.insert(fixtureDefs.end(), defs.begin(), defs.end());
				for(auto def : defs) {
					map_fixtureDefDeviceComp.insert({ def, pComp });
				}

				map_EDCompDeviceComp.insert({ pEDComp, pComp });
				m_deviceComponents.insert(pComp);
			} else {
				Frame::gEntitySystem->RemoveEntity(pEntity->GetId());
			}
		}
	}

	for(const auto & [pEDComp, pComp] : map_EDCompDeviceComp) {
		for(int i = 0; i < 4; i++) {
			if(!pEDComp->m_neighbors[i]) {
				continue;
			}

			if(auto it = map_EDCompDeviceComp.find(pEDComp->m_neighbors[i]); it != map_EDCompDeviceComp.end()) {
				pComp->WeldWith(it->second, i);
			}
		}
	}

	/* -------------------- 物理化 -------------------- */
	
	{
		const Frame::Vec2 entPos = PixelToMeterVec2(m_pEntity->GetPosition());

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(entPos.x, entPos.y);
		bodyDef.angle = Frame::DegToRad(m_pEntity->GetRotation());
		bodyDef.linearDamping = 1.f;
		bodyDef.angularDamping = 1.f;

		m_pRigidbodyComponent->Physicalize(bodyDef, fixtureDefs, map_fixtureDefDeviceComp);
		//m_pRigidbodyComponent->SetEnableRendering(true);
	}

	/* --------------------- 创建管道 --------------------- */

	std::unordered_map<SEditorPipeNode *, SPipeNode *> map_EPNodePNode;

	for(const auto & editorPipe : pipes) {
		std::unordered_set<SPipeNode *> pipe;
		for(const auto & pEditorPipeNode : editorPipe) {
			SPipeNode * pPipeNode = new SPipeNode { pEditorPipeNode->pos };
			pPipeNode->dirIndexForDevice = pEditorPipeNode->dirIndexForDevice;
			if(pEditorPipeNode->pDevice) {
				pPipeNode->pDevice = map_EDCompDeviceComp[pEditorPipeNode->pDevice];
				pPipeNode->pDevice->GetPipeNodes().insert(pPipeNode);
			}
			pipe.insert(pPipeNode);
			map_EPNodePNode.insert({ pEditorPipeNode, pPipeNode });
		}
		m_pipes.push_back(pipe);
	}

	for(const auto & [pEPNode, pPipeNode] : map_EPNodePNode) {
		for(int i = 0; i < 4; i++) {
			if(pEPNode->nodes[i]) {
				pPipeNode->nodes[i] = map_EPNodePNode[pEPNode->nodes[i]];
			}
		}
	}

	/* --------------------- 分组装置 --------------------- */

	std::unordered_set<SPipeNode *> pipeNodesRecursived;
	for(const auto & pipe : m_pipes) {
		bool bAlreadyHasGroup = false;
		for(const auto & pPipeNode : pipe) {
			if(pPipeNode->pDevice && pPipeNode->pDevice->GetGroup()) {
				bAlreadyHasGroup = true;
				break;
			}
		}
		if(bAlreadyHasGroup) {
			continue;
		}

		std::vector<SPipeNode *> pipeNodesOnDevice;
		PipeRecursion(& pipeNodesRecursived, * pipe.begin(), 2, & pipeNodesOnDevice);

		SGroup * pGroup = new SGroup {};
		for(auto & pPipeNode : pipeNodesOnDevice) {
			pGroup->InsertAndBind(pPipeNode->pDevice);
		}
		m_groups.insert(pGroup);
	}

	/* --------------------------------------------------- */

	m_pDeviceConnectorRendererEntity = Frame::gEntitySystem->SpawnEntity();
	if(m_pDeviceConnectorRendererEntity) {
		if(CDeviceConnectorRendererComponent * pDeviceConnectorRendererComponent = m_pDeviceConnectorRendererEntity->CreateComponent<CDeviceConnectorRendererComponent>()) {
			pDeviceConnectorRendererComponent->Initialize(& m_deviceComponents);
		}
	}
}

void CMachineComponent::OnShutDown() {
	// TODO - 清除所有 SPipeNode
	// TODO - 清除所有 SGroup

	if(m_pDeviceConnectorRendererEntity) {
		Frame::gEntitySystem->RemoveEntity(m_pDeviceConnectorRendererEntity->GetId());
	}
}

void CMachineComponent::SGroup::InsertAndBind(CDeviceComponent * pDeviceComp) {
	if(!pDeviceComp) {
		return;
	}
	auto pNode = pDeviceComp->GetNode();
	if(!pNode || !pNode->pDeviceData) {
		return;
	}
	(pNode->pDeviceData->device == IDeviceData::Engine ? engines : devices).insert(pDeviceComp);
	pDeviceComp->SetGroup(this);
}
