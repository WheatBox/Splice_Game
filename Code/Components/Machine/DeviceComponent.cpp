#include "DeviceComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../../Utility.h"
#include "../../Assets.h"
#include "../../Depths.h"
#include "../../Pipe.h"

#include "../PhysicsWorldComponent.h"
#include "../SmokeEmitterComponent.h"

REGISTER_ENTITY_COMPONENT(CDeviceComponent);

std::unordered_set<CDeviceComponent *> CDeviceComponent::s_workingDevices;

Frame::EntityEvent::Flags CDeviceComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::BeforeUpdate
		| Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::AfterUpdate
		| Frame::EntityEvent::EFlag::Render
		;
}

void CDeviceComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::BeforeUpdate:
		if(m_pGroup) {
			m_pGroup->bEngineWorkingPrevFrame = m_pGroup->bEngineWorking;
		}
		break;
	case Frame::EntityEvent::Update:
		m_frametime = event.params[0].f;

		break;
	case Frame::EntityEvent::AfterUpdate:
	{
		if(m_pMachinePartEntity) {
			const float machineRot = m_pMachinePartEntity->GetRotation();
			m_pEntity->SetPosition(m_pMachinePartEntity->GetPosition() + m_relativePosition.GetRotated(machineRot));
			m_pEntity->SetRotation(machineRot + m_relativeRotation);
			// 关于机器部分(MachinePart)实体的坐标与旋转，见 CMachinePartComponent 的注释
		}

		if(m_keyId != Frame::EKeyId::eKI_Unknown && m_pGroup) {
			if(Frame::gInput->pKeyboard->GetPressed(m_keyId)) {
				m_pGroup->bEngineWorking = !m_pGroup->bEngineWorkingPrevFrame;
			}
		}
	}
	break;
	case Frame::EntityEvent::EFlag::Render:

		//Frame::gRenderer->pTextRenderer->DrawText("------------------------------------" + std::to_string(m_relativePosition.x) + ", " + std::to_string(m_relativePosition.y) + ",,, " + std::to_string(m_relativeRotation), m_pEntity->GetPosition());
		//Frame::gRenderer->pTextRenderer->DrawText("\n------------------------------------" + std::to_string(m_pMachinePartEntity->GetPosition().x) + ", " + std::to_string(m_pMachinePartEntity->GetPosition().y) + ",,, " + std::to_string(m_pMachinePartEntity->GetRotation()), m_pEntity->GetPosition());
		
		//Frame::gRenderer->pTextRenderer->DrawTextBlended(std::to_string((size_t)m_pGroup), m_pEntity->GetPosition(), 0x000000, 1.f);

#if 1
		switch(GetDeviceType()) {
		case IDeviceData::Joint:
		{
			auto pData = reinterpret_cast<SJointDeviceData *>(m_pNode->pDeviceData);
			if(CDeviceComponent * pAnotherComp = pData->GetBehindMachinePartDeviceComponent()) {
				Frame::gRenderer->pShapeRenderer->DrawPointBlended(m_pEntity->GetPosition() + 64.f, 0x000000, 1.f, 16.f);
				Frame::gRenderer->pTextRenderer->DrawTextBlended("FULL!", m_pEntity->GetPosition() + 64.f, 0x000000, 1.f);
			} else {
				Frame::gRenderer->pShapeRenderer->DrawPointBlended(m_pEntity->GetPosition() + 64.f, 0x00FF00, 1.f, 16.f);
				Frame::gRenderer->pTextRenderer->DrawTextBlended("GOOD!!", m_pEntity->GetPosition() + 64.f, 0x000000, 1.f);
			}
		}
		break;
		}
#endif
		break;
	}
}

void CDeviceComponent::Initialize(Frame::CEntity * pMachinePartEntity, IDeviceData::EType deviceType, Frame::EKeyId keyId, int dirIndex, const SColorSet & colorSet) {
	m_pEntity->SetZDepth(Depths::Device);

	m_pMachinePartEntity = pMachinePartEntity;

	m_pNode = new SDeviceTreeNode { deviceType };

	m_directionIndex = dirIndex;
	m_colorSet = colorSet;

	m_keyId = keyId;

	m_pSpriteComponent = m_pEntity->CreateComponent<CSpriteComponent>();
	m_pNode->pDeviceData->InitSprite(m_pSpriteComponent, this, colorSet);

	//switch(deviceType) {
	//case IDeviceData::Joint:
		// 写在 CMachineComponent::Initialize() 里了
		//break;
	//}

	s_workingDevices.insert(this);
}

void CDeviceComponent::OnShutDown() {
	if(auto it = s_workingDevices.find(this); it != s_workingDevices.end()) {
		s_workingDevices.erase(it);
	}

	if(CMachinePartComponent * pMachinePartComp = m_pMachinePartEntity->GetComponent<CMachinePartComponent>()) {
		pMachinePartComp->OnDeviceShutDown(this, m_pipeNodes, m_pGroup);
	}
	
	delete m_pNode;
	m_pNode = nullptr;
}

std::vector<std::pair<b2ShapeDef, CRigidbodyComponent::SBox2dShape>> CDeviceComponent::MakeShapeDefs() {
	if(!m_pNode) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "Trying to call CDeviceComponent::MakeShapeDefs(), but m_pNode is nullptr!");
		return {};
	}
	if(!m_pNode->pDeviceData) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "Trying to call CDeviceComponent::MakeShapeDefs(), but m_pNode->pDeviceData is nullptr!");
		return {};
	}
	return m_pNode->pDeviceData->MakeShapeDefs(m_pEntity->GetPosition(), m_pEntity->GetRotation());
}

void CDeviceComponent::WeldWith(CDeviceComponent * pDeviceComp, int dirIndex) {
	if(!pDeviceComp) {
		return;
	}

	SDeviceTreeNode * pAnotherNode = pDeviceComp->GetNode();

	if(!m_pNode || !m_pNode->pDeviceData || !pAnotherNode || !pAnotherNode->pDeviceData) {
		return;
	}

	if(m_pNode->nodes[dirIndex]) {
		return;
	}

	for(const auto & _pNode : m_pNode->nodes) {
		if(_pNode == pAnotherNode) {
			return;
		}
	}

	m_pNode->nodes[dirIndex] = pAnotherNode;
	pAnotherNode->nodes[GetRevDirIndex(dirIndex)] = m_pNode;
}

static void __DrawConnector(const Frame::Vec2 & entityPos, const SDeviceTreeNode * m_pNode, int m_directionIndex, int drawDirIndex, float rot, const SColorSet & m_colorSet) {
	Frame::Vec2 pos = entityPos
		+ GetDeviceInterfaceBias(m_pNode->pDeviceData->device, m_directionIndex, drawDirIndex, rot)
		+ GetRectangleEdgePosByDirIndex(GetDevicePixelSize(m_pNode->pDeviceData->device) + CONNECTOR_HALF_LENGTH * 2.f, m_directionIndex, drawDirIndex).GetRotated(rot)
		;
	Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::connector)->GetImage(), pos, m_colorSet.connector, 1.f,
		{ 1.f }, drawDirIndex * Frame::DegToRad(90.f) + rot
	);
}

void CDeviceComponent::DrawConnectors() const {
	if(!m_pNode || !m_pNode->pDeviceData) {
		return;
	}

	const Frame::Vec2 entityPos = m_pEntity->GetPosition();
	const float entityRot = m_pEntity->GetRotation();
	const float baseRot = entityRot - m_relativeRotation;

	if(IsMachinePartJoint(m_pNode->pDeviceData->device)) {
		goto ForMachinePartJoints;
	}

	for(int i = 0; i < 2; i++) {
		if(!m_pNode->nodes[i]) {
			continue;
		}
		__DrawConnector(entityPos, m_pNode, m_directionIndex, i, baseRot, m_colorSet);
	}

	return;
ForMachinePartJoints:

	for(int i = 0; i < 4; i++) {
		float rot = baseRot;

		//auto pData = reinterpret_cast<SDeviceDataMachinePartJoint *>(reinterpret_cast<char *>(m_pNode->pDeviceData) + sizeof(IDeviceData));
		auto pData = reinterpret_cast<SJointDeviceData *>(m_pNode->pDeviceData);

		CDeviceComponent * pAnotherDevice = nullptr;
		if(i < 2 && i == GetRevDirIndex(GetDirIndex())) {
			pAnotherDevice = pData->GetBehindMachinePartDeviceComponent();
		} else if(i == pData->GetPointDirIndex()) {
			pAnotherDevice = pData->GetPointMachinePartDeviceComponent();
			if(pAnotherDevice) {
				rot += (pAnotherDevice->GetEntity()->GetPosition() - entityPos).Radian() - entityRot + pData->GetRotationAdd();
			}
		}

		if(!pAnotherDevice) {
			continue;
		}

		if(i >= 2) {
			if(IsMachinePartJoint(pAnotherDevice->GetDeviceType())) {
				continue;
			}
		}

		__DrawConnector(entityPos, m_pNode, m_directionIndex, i, rot, m_colorSet);
	}
}

void CDeviceComponent::Step(float timeStep, float power, void * userdata) {
	if(!m_pNode || !m_pNode->pDeviceData || !m_pMachinePartEntity) {
		return;
	}
	const IDeviceData::EType deviceType = m_pNode->pDeviceData->device;

	switch(deviceType) {
	case IDeviceData::Propeller:
		if(auto pRigidbodyComp = m_pMachinePartEntity->GetComponent<CRigidbodyComponent>()) {
			pRigidbodyComp->ApplyForce(
				Frame::Vec2 { -1200.f * power, 0.f }.GetRotated(m_pEntity->GetRotation())
				, m_relativePosition.GetRotated(m_pMachinePartEntity->GetRotation()) + m_pMachinePartEntity->GetPosition()
			);
		}
		const float rot = m_pSpriteComponent->GetLayers()[3].GetRotationDegree() + (1000.f + 600.f * power) * timeStep;
		m_pSpriteComponent->GetLayers()[3].SetRotationDegree(rot);
		m_pSpriteComponent->GetLayers()[4].SetRotationDegree(rot);
		break;
	}

	timeStep, userdata;
}

#if 0
void CDeviceComponent::Step(float timeStep) {
	if(!m_pNode || !m_pNode->pDeviceData || !m_pMachinePartEntity) {
		return;
	}

	const IDeviceData::EType deviceType = m_pNode->pDeviceData->device;
	
	///////////////////////
	// TODO - 这部分就随便一写，仅作为临时测试用，包括也没考虑多人情况d
	if(deviceType == IDeviceData::EType::Cabin) {
		if(CCameraComponent * pCamComp = CPhysicsWorldComponent::s_pPhysicsWorldComponent->GetCameraComponent()) {
			pCamComp->FollowEntity(m_pEntity, true);
			//pCamComp->SetFollowRotation(true);
		}
	}
	///////////////////////

	if(deviceType == IDeviceData::Engine) { // 这段代码是测试烟雾特效用的，作用是让所有的发动机装置无需发动就自动冒烟
		SEngineDeviceData * pData = reinterpret_cast<SEngineDeviceData *>(m_pNode->pDeviceData);
		pData->smoking += m_frametime;
		if(pData->smoking > 0.f && pData->smoking < 100.f) { // 防止意外的死循环
			while(pData->smoking >= pData->smokeMax) {
				CSmokeEmitterComponent::SummonSmokeParticle({ m_pEntity->GetPosition(), 1.f });
				pData->smoking -= pData->smokeMax;
			}
		} else {
			pData->smoking = 0.f;
		}
		return;
	}
	
	if(!m_pGroup) {
		goto ForThoseDevicesHasNoGroup;
	}

	{
		/* ---------------------- 计算动力 ---------------------- */

		bool working = false;
		float power = 0.f;
		if(m_pGroup->bEngineWorking) {
			power = m_pGroup->devices.size() != 0 ? (static_cast<float>(m_pGroup->engines.size()) / static_cast<float>(m_pGroup->devices.size())) : 0.f;
			working = true;
		}

		// _minPower = 最低运行动力，低于该动力就不会运行
		// _maxPower = 最高支持动力，高于该动力的动力会被浪费掉
	#define __FORMULA(_minPower, _maxPower) if(power < _minPower) { power = 0.f; working = false; } else if(power > _maxPower) power = _maxPower; break;

		switch(deviceType) {
		case IDeviceData::Propeller: __FORMULA(.4f, 4.f);
		case IDeviceData::JetPropeller: __FORMULA(.1f, 2.f);
		}

	#undef __FORMULA

		/* ---------------------- 运行装置 ---------------------- */

		switch(deviceType) {
		case IDeviceData::Propeller:
			if(!working) {
				break;
			}
			if(auto pRigidbodyComp = m_pMachinePartEntity->GetComponent<CRigidbodyComponent>()) {
				pRigidbodyComp->ApplyForce(
					Frame::Vec2 { -1200.f * power, 0.f }.GetRotated(m_pEntity->GetRotation())
					, m_relativePosition.GetRotated(m_pMachinePartEntity->GetRotation()) + m_pMachinePartEntity->GetPosition()
				);
			}
			break;
		case IDeviceData::JetPropeller:
		{
			SJetPropellerDeviceData * pData = reinterpret_cast<SJetPropellerDeviceData *>(m_pNode->pDeviceData);
			if(pData->accumulating < 0.f) {
				pData->accumulating += timeStep * 1.f;
				if(pData->accumulating > 0.f) {
					pData->accumulating = 0.f;
				}
			} else {
				pData->accumulating += timeStep * power;
			}
			if(pData->accumulating >= pData->accumulationMax) {
				if(auto pRigidbodyComp = m_pMachinePartEntity->GetComponent<CRigidbodyComponent>()) {
					pRigidbodyComp->ApplyLinearImpulse(
						Frame::Vec2 { -6000.f * power, 0.f }.GetRotated(m_pEntity->GetRotation())
						, m_relativePosition.GetRotated(m_pMachinePartEntity->GetRotation()) + m_pMachinePartEntity->GetPosition()
					);
				}
				pData->accumulating = -.5f;
			}
			pData->accumulatingShowingPrev = pData->accumulatingShowing;
			pData->accumulatingShowing = Lerp(pData->accumulatingShowing, std::max(pData->accumulating, 0.f), timeStep * 10.f);
		}
		break;
		}

		/* ---------------------- 其它功能 ---------------------- */

		switch(deviceType) {
		case IDeviceData::Engine:
		{
			if(!working) {
				break;
			}
			SEngineDeviceData * pData = reinterpret_cast<SEngineDeviceData *>(m_pNode->pDeviceData);
			pData->smoking += timeStep;
			if(pData->smoking > 0.f && pData->smoking < 100.f) { // 防止意外的死循环
				while(pData->smoking >= pData->smokeMax) {
					CSmokeEmitterComponent::SummonSmokeParticle({ m_pEntity->GetPosition(), 1.f });
					pData->smoking -= pData->smokeMax;
				}
			} else {
				pData->smoking = 0.f;
			}
		}
		break;
		case IDeviceData::Propeller:
		{
			if(!working) {
				break;
			}
			const float rot = m_pSpriteComponent->GetLayers()[3].GetRotationDegree() + (1000.f + 600.f * power) * timeStep;
			m_pSpriteComponent->GetLayers()[3].SetRotationDegree(rot);
			m_pSpriteComponent->GetLayers()[4].SetRotationDegree(rot);
		}
		break;
		case IDeviceData::JetPropeller:
		{
			SJetPropellerDeviceData * pData = reinterpret_cast<SJetPropellerDeviceData *>(m_pNode->pDeviceData);
			const float rot = Frame::DegToRad(45.f)
				+ Frame::DegToRad(270.f) * Frame::Clamp(pData->accumulatingShowing, 0.f, pData->accumulationShowingMax) / pData->accumulationShowingMax
				+ (pData->accumulatingShowing <= pData->accumulationShowingMax ? 0.f : Frame::DegToRad(12.f) * -std::sin(Frame::DegToRad(30.f) * (pData->accumulationShowingMax - pData->accumulatingShowing)))
				;
			m_pSpriteComponent->GetLayers()[4].SetRotationDegree(rot);

			if(pData->accumulatingShowing > .002f && pData->accumulating < 0.f) {
				for(int i = 0; i < 4; i++) {
					CSmokeEmitterComponent::SSmokeParticle particle {
						m_pEntity->GetPosition() + Frame::Vec2 { 96.f, 0.f }.GetRotated(m_pEntity->GetRotation()),
						1.f, 0xFFFFFF, Frame::Vec2 { 4000.f, 0.f }.GetRotated(m_pEntity->GetRotation() + static_cast<float>(rand() % 41 - 20))
					};
					particle.alpha *= -pData->accumulating * 3.f;
					particle.scaleAdd *= 5.f;
					particle.alphaAdd *= 1.5f;
					CSmokeEmitterComponent::SummonSmokeParticle(particle);
				}
			}
		}
		break;
		}

		/* ----------------------------------------------------- */
	}

	return;
ForThoseDevicesHasNoGroup:

	switch(deviceType) {
	case IDeviceData::Joint:
	{
		auto pData = reinterpret_cast<SJointDeviceData *>(m_pNode->pDeviceData);
		if(CDeviceComponent * pAnotherComp = pData->GetPointMachinePartDeviceComponent()) {
			if(const Frame::Vec2 anotherPos = pAnotherComp->GetEntity()->GetPosition(), myPos = m_pEntity->GetPosition(); anotherPos != myPos) {
				float rot = (anotherPos - myPos).Degree() - m_pEntity->GetRotation() + pData->GetRotationAdd();
				m_pSpriteComponent->GetLayers()[0].SetRotationDegree(rot);
				m_pSpriteComponent->GetLayers()[3].SetRotationDegree(rot);
				m_pSpriteComponent->GetLayers()[4].SetRotationDegree(rot);
			}
		}
	}
	break;
	}
}
#endif