#include "MachinePartComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>

#include "DeviceComponent.h"
#include "../Editor/EditorDeviceComponent.h"
#include "DeviceConnectorRendererComponent.h"
#include "../RigidbodyComponent.h"

#include "../../Depths.h"
#include "../../Pipe.h"

#include <algorithm>

REGISTER_ENTITY_COMPONENT(CMachinePartComponent);

Frame::EntityEvent::Flags CMachinePartComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::Render;
}

void CMachinePartComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Update:
		if(m_pRigidbodyComponent) {
			m_pEntity->SetPosition(m_pRigidbodyComponent->GetPosition());
			m_pEntity->SetRotation(m_pRigidbodyComponent->GetRotation());
		}
		break;
	case Frame::EntityEvent::Render:
		//for(const auto & pipe : m_pipes) {
			// DrawPipe<SPipeNode>(pipe, m_pEntity->GetPosition(), m_colorSet.pipe, 1.f, m_pEntity->GetRotation());
			// TODO
		//}
		break;
	}
}

void CMachinePartComponent::Initialize(std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> * out_map_EDCompDeviceComp_or_nullptr, CMachineComponent * pMachine, const std::unordered_set<CEditorDeviceComponent *> & editorDeviceComps, const std::unordered_set<const std::vector<SEditorPipeNode *> *> & pipes, const SColorSet & colorSet) {

	m_pMachineBelonging = pMachine;

	m_pEntity->SetZDepth(Depths::Machine);

	m_colorSet = colorSet;

	m_pRigidbodyComponent = m_pEntity->CreateComponent<CRigidbodyComponent>();
	std::vector<std::pair<b2ShapeDef, CRigidbodyComponent::SBox2dShape>> shapeDefs;

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

				//pComp->Initialize(m_pEntity, pEDComp->GetDeviceType(), pEDComp->GetKeyId(), pEDComp->GetDirIndex(), colorSet); // TODO
				pComp->SetRelativePositionRotation(devicePos, deviceRot);

				auto defs = CDeviceComponent::MakeShapeDefs(pEDComp->GetDeviceType(), devicePos, deviceRot);
				shapeDefs.insert(shapeDefs.end(), defs.begin(), defs.end());

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

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = { entPos.x, entPos.y };
		bodyDef.rotation = b2MakeRot(Frame::DegToRad(m_pEntity->GetRotation()));
		bodyDef.linearDamping = 1.f;
		bodyDef.angularDamping = 1.f;

		m_pRigidbodyComponent->Physicalize(bodyDef, shapeDefs);
		//m_pRigidbodyComponent->SetEnableRendering(true);
	}

	/* --------------------- 创建管道 --------------------- */

	std::unordered_map<SEditorPipeNode *, SPipeNode *> map_EPNodePNode;
	
	std::unordered_set<SPipeNode *> wrongPipeNodes; // 具体信息见下方的注释【关于 wrongPipeNodes】

	for(const auto & editorPipe : pipes) {
		std::unordered_set<SPipeNode *> pipe;

		for(const auto & pEditorPipeNode : * editorPipe) {
			SPipeNode * pPipeNode = new SPipeNode { pEditorPipeNode->pos };
			pPipeNode->dirIndexForDevice = pEditorPipeNode->dirIndexForDevice;
			if(pEditorPipeNode->pDevice) {
				if(auto it = map_EDCompDeviceComp.find(pEditorPipeNode->pDevice); it != map_EDCompDeviceComp.end()) {
					pPipeNode->pDevice = it->second;
					pPipeNode->pDevice->GetPipeNodes().insert(pPipeNode);
				} else {
					wrongPipeNodes.insert(pPipeNode);
				}
			}
			pipe.insert(pPipeNode);
			map_EPNodePNode.insert({ pEditorPipeNode, pPipeNode });
		}

		m_pipes.push_back(pipe);
	}

	for(const auto & [pEPNode, pPipeNode] : map_EPNodePNode) {
		for(int i = 0; i < 4; i++) {
			pPipeNode->nodes[i] = pEPNode->nodes[i] ? map_EPNodePNode[pEPNode->nodes[i]] : nullptr;
		}
	}

	// 【关于 wrongPipeNodes】
	// 若管道或管道的某条分支连接到的装置在机器部分上
	// 则该条管道或该分支就是错误的，因为 editorDeviceComps 上存储的只有当前机器部分的装置
	// 但是 editorPipe 中存储着另一机器部分的装置，也就是 editorDeviceComps 中不存在的装置
	// 所以这条管道或该分支和相关的节点都需要被清除
	{
		size_t safe = 0;
		while(!wrongPipeNodes.empty() && safe != wrongPipeNodes.size()) {
			safe = wrongPipeNodes.size();

			std::unordered_set<SPipeNode *> erasingNodes;
			PipeRecursion(& erasingNodes, * wrongPipeNodes.begin(), 1);

			// bool wrongPipeNodeErased = false;

			for(auto & erasingNode : erasingNodes) {
				for(auto itPipes = m_pipes.begin(); itPipes != m_pipes.end();) {
					if(auto it = itPipes->find(erasingNode); it != itPipes->end()) {
						itPipes->erase(it);
						if(itPipes->empty()) {
							itPipes = m_pipes.erase(itPipes);
							continue;
						}
					}
					itPipes++;
				}
				if(auto it = wrongPipeNodes.find(erasingNode); it != wrongPipeNodes.end()) {
					wrongPipeNodes.erase(it);
					// wrongPipeNodeErased = true;
				}
				delete erasingNode;
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

	if(out_map_EDCompDeviceComp_or_nullptr) {
		* out_map_EDCompDeviceComp_or_nullptr = map_EDCompDeviceComp;
	}
}

void CMachinePartComponent::OnShutDown() {
	if(m_pDeviceConnectorRendererEntity) {
		Frame::gEntitySystem->RemoveEntity(m_pDeviceConnectorRendererEntity->GetId());
	}

	for(auto & pDeviceComp : m_deviceComponents) {
		Frame::gEntitySystem->RemoveEntity(pDeviceComp->GetEntity()->GetId());
	}
	// 正常来说，m_groups 和 m_pipes 里的东西应该在上面这一步就清理完了
	// 但是以防万一也许代码某个地方有遗漏，所以依然写了下面这些
	for(auto & pGroup : m_groups) {
		delete pGroup;
	}
	for(auto & pipe : m_pipes) {
		for(auto & pPipeNode : pipe) {
			delete pPipeNode;
		}
	}
}

void CMachinePartComponent::OnDeviceShutDown(CDeviceComponent * pDeviceComp, const std::unordered_set<SPipeNode *> & pipeNodes, SGroup * pGroup) {
	if(!pDeviceComp) {
		return;
	}

	if(pGroup) {
		std::unordered_set<CDeviceComponent *> & refDeviceSet = pDeviceComp->GetDeviceType() == IDeviceData::EType::Engine ? pGroup->engines : pGroup->devices;
		if(auto it = refDeviceSet.find(pDeviceComp); it != refDeviceSet.end()) {
			refDeviceSet.erase(it);

			if(pGroup->engines.empty() && pGroup->devices.empty()) {
				m_groups.erase(pGroup);
				delete pGroup;
			}
		}
	}

	std::unordered_set<SPipeNode *> erasingPipeNodes;
	for(auto & pPipeNode : pipeNodes) {
		PipeRecursion(& erasingPipeNodes, pPipeNode, 1);
	}
	for(auto & pPipeNode : erasingPipeNodes) {
		for(auto it = m_pipes.begin(); it != m_pipes.end();) {
			if(it->find(pPipeNode) != it->end()) {
				it->erase(pPipeNode);
				if(it->empty()) {
					it = m_pipes.erase(it);
					continue;
				}
			}
			it++;
		}
		delete pPipeNode;
	}

	// TODO - 从 m_pRigidbodyComponent 中移除相应的 b2Fixture
	// TODO - 所有装置都没了的话销毁该机器部分
	// TODO - 若破损程度已经使得该机器部分分成多个独立的部分，则产生新的机器部分，记得考虑机器部分脱离机器的情况
	// TODO - 上一条 TODO 中，也许会对原本固定住的关节、活塞等得以活动，应当考虑此类情况（备注：并不会为“原本固定住的关节、活塞等”创建它们的 b2Joint）
}

bool CMachinePartComponent::CreateJointWith(CMachinePartComponent * pAnotherMachinePartComp, CDeviceComponent * pJointComp) {
	if(!m_pRigidbodyComponent || !pAnotherMachinePartComp || !pJointComp) {
		return false;
	}

	bool bRes = false;

	switch(pJointComp->GetDeviceType()) {
	case IDeviceData::EType::Joint:
	{
		Frame::Vec2 anchor = PixelToMeterVec2(pJointComp->GetEntity()->GetPosition());
		bRes = m_pRigidbodyComponent->CreateJointWith(pAnotherMachinePartComp->GetEntity()->GetId(),
			[anchor](b2BodyId myBody, b2BodyId anotherBody) {
				b2RevoluteJointDef def = b2DefaultRevoluteJointDef();
				
				def.bodyIdA = myBody;
				def.bodyIdB = anotherBody;
				def.localAnchorA = b2Body_GetLocalPoint(myBody, { anchor.x, anchor.y });
				def.localAnchorB = b2Body_GetLocalPoint(anotherBody, { anchor.x, anchor.y });

				def.collideConnected = true;
				def.referenceAngle = Frame::DegToRad(0.f);
				def.lowerAngle = Frame::DegToRad(-90.f);
				def.upperAngle = Frame::DegToRad(90.f);
				def.enableLimit = true;

				b2CreateRevoluteJoint(gWorldId, & def);
			}
		);
	}
	break;
	}

	return bRes;
}

void CMachinePartComponent::SGroup::InsertAndBind(CDeviceComponent * pDeviceComp) {
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
