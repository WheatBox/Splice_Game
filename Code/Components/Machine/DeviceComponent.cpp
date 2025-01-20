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
		}

		if(m_keyId != Frame::EKeyId::eKI_Unknown && m_pGroup) {
			if(Frame::gInput->pKeyboard->GetPressed(m_keyId)) {
				m_pGroup->bEngineWorking = !m_pGroup->bEngineWorkingPrevFrame;
			}
		}
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
		//Frame::gRenderer->pTextRenderer->DrawTextBlended(std::to_string((size_t)m_pGroup), m_pEntity->GetPosition(), 0x000000, 1.f);

#if 0
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
	Frame::Log::Log(Frame::Log::ELevel::Debug, "device init start");
	if(deviceType <= IDeviceData::EType::Unset || deviceType >= IDeviceData::EType::END) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "device init unknown device type");
		return;
	}

	m_pEntity->SetZDepth(Depths::Device);

	m_pMachinePartEntity = pMachinePartEntity;

	m_pNode = new SDeviceTreeNode { deviceType };

	m_directionIndex = dirIndex;
	m_colorSet = colorSet;

	m_keyId = keyId;

#define __SPRITE_ALPHA 1.f // 为了方便调试，放了这么个宏在这里

#define __ADD_SPRITE_LAYER(__EDeviceStaticSprite) \
m_pSpriteComponent->AddLayer({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA });
#define __ADD_SPRITE_LAYER_EXT(__EDeviceStaticSprite, __colorSetMemberVariable) \
m_pSpriteComponent->AddLayer({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), colorSet.__colorSetMemberVariable, __SPRITE_ALPHA });

#define __ADD_SPRITE_LAYER_GROUPED(__EDeviceStaticSprite, __group) \
m_pSpriteComponent->AddLayer({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA }, __group);
#define __ADD_SPRITE_LAYER_EXT_GROUPED(__EDeviceStaticSprite, __colorSetMemberVariable, __group) \
m_pSpriteComponent->AddLayer({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), colorSet.__colorSetMemberVariable, __SPRITE_ALPHA }, __group);

	m_pSpriteComponent = m_pEntity->CreateComponent<CSpriteComponent>();

	if(deviceType == IDeviceData::Cabin) {
		__ADD_SPRITE_LAYER(cabin_logo_background);
	} else if(deviceType == IDeviceData::JetPropeller) {
		__ADD_SPRITE_LAYER_EXT(jet_propeller_bottom, color1);
	} else if(deviceType == IDeviceData::Joint) {
		__ADD_SPRITE_LAYER_EXT(joint_bottom, color2);
	}

	m_pSpriteComponent->AddLayer({ Assets::GetDeviceStaticSprite(deviceType, Assets::EDeviceStaticSpritePart::color1), colorSet.color1, __SPRITE_ALPHA });
	if(deviceType != IDeviceData::Joint) {
		m_pSpriteComponent->AddLayer({ Assets::GetDeviceStaticSprite(deviceType, Assets::EDeviceStaticSpritePart::color2), colorSet.color2, __SPRITE_ALPHA });
	}
	m_pSpriteComponent->AddLayer({ Assets::GetDeviceStaticSprite(deviceType, Assets::EDeviceStaticSpritePart::basic), 0xFFFFFF, __SPRITE_ALPHA });

	if(deviceType == IDeviceData::Propeller) {
		__ADD_SPRITE_LAYER_EXT_GROUPED(propeller_blade_color, color2, dynamicInsBufferGroupIndex);
		{
			auto & layerTemp = m_pSpriteComponent->GetLayers().back();
			layerTemp.SetScale({ .3f, 1.f });
			layerTemp.SetOffset({ -20.f, 0.f });
			layerTemp.SetRotationDegree(30.f);
		}
		__ADD_SPRITE_LAYER_GROUPED(propeller_blade, dynamicInsBufferGroupIndex);
		{
			auto & layerTemp = m_pSpriteComponent->GetLayers().back();
			layerTemp.SetScale({ .3f, 1.f });
			layerTemp.SetOffset({ -20.f, 0.f });
			layerTemp.SetRotationDegree(30.f);
		}

		__ADD_SPRITE_LAYER_EXT_GROUPED(propeller_top_color, color1, staticTopInsBufferGroupIndex);
		__ADD_SPRITE_LAYER_GROUPED(propeller_top, staticTopInsBufferGroupIndex);
	} else if(deviceType == IDeviceData::JetPropeller) {
		__ADD_SPRITE_LAYER(jet_propeller_needle);
		{
			auto & layerTemp = m_pSpriteComponent->GetLayers().back();
			layerTemp.SetOffset({ -32.f, 20.f });
			layerTemp.SetRotationDegree(45.f);
		}
	} else if(deviceType == IDeviceData::Joint) {
		m_pSpriteComponent->GetLayers()[1].SetRotationDegree(180.f);
		m_pSpriteComponent->GetLayers()[2].SetRotationDegree(180.f);
		__ADD_SPRITE_LAYER_EXT(joint_top_color, color2);
		__ADD_SPRITE_LAYER(joint_top);
	}

#undef __SPRITE_ALPHA
#undef __ADD_SPRITE_LAYER
#undef __ADD_SPRITE_LAYER_EXT
#undef __ADD_SPRITE_LAYER_GROUPED
#undef __ADD_SPRITE_LAYER_EXT_GROUPED

	switch(deviceType) {
	case IDeviceData::JetPropeller:
		m_pSpriteComponent->GetLayers()[0].SetExtraFunction([this](float frameTime) {
			if(!m_pNode || !m_pNode->pDeviceData) {
				return;
			}

			auto pData = reinterpret_cast<SJetPropellerDeviceData *>(m_pNode->pDeviceData);

			const Frame::Vec2 entPos = m_pEntity->GetPosition();
			const float entRot = m_pEntity->GetRotation();

			const Frame::Vec2 pos = entPos - Frame::Vec2 { 32.f, 0.f }.GetRotated(entRot);
			const float alpha = std::min(1.f, pData->accumulatingShowing / pData->accumulationShowingMax) * .35f;

			pData->smokeRotation1 += frameTime * (40.f + 30000.f * (pData->accumulatingShowing - pData->accumulatingShowingPrev));
			pData->smokeRotation2 += frameTime * (80.f + 60000.f * (pData->accumulatingShowing - pData->accumulatingShowingPrev));

			Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::jet_propeller_bottom)->GetImage(),
				entPos, 0xFFFFFF, alpha * 2.5f, 1.f, entRot
			);
			for(int i = 0; i < CSmokeEmitterComponent::SSmokeParticle::spritesCount; i++) {
				Frame::gRenderer->DrawSpriteBlended(
					Assets::GetStaticSprite(CSmokeEmitterComponent::SSmokeParticle::sprites[i])->GetImage(),
					pos + Frame::Vec2 { 16.f, 0.f }.GetRotatedDegree(static_cast<float>(i * (360 / CSmokeEmitterComponent::SSmokeParticle::spritesCount)) + pData->smokeRotation1), 0xFFFFFF, alpha,
					{ .75f }, Frame::DegToRad(pData->smokeRotation2)
				);
			}
		});
		break;
	//case IDeviceData::Joint:
		// 写在 CMachineComponent::Initialize() 里了
		//break;
	}

	s_workingDevices.insert(this);
	Frame::Log::Log(Frame::Log::ELevel::Debug, "device init end");
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
				rot += (pAnotherDevice->GetEntity()->GetPosition() - entityPos).Degree() - entityRot + pData->GetRotationAdd();
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

#define F(x) PixelToMeter(x)

std::vector<std::pair<b2ShapeDef, CRigidbodyComponent::SBox2dShape>> CDeviceComponent::MakeShapeDefs(IDeviceData::EType deviceType, const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, CRigidbodyComponent::SBox2dShape>> defs;

#define __NEW_DEF(_density, _friction, _restitution) {\
	b2ShapeDef defTemp = b2DefaultShapeDef(); \
	defTemp.density = _density; \
	defTemp.friction = _friction; \
	defTemp.restitution = _restitution; \
	defs.push_back({ defTemp, {} }); \
}

	constexpr float shellDensity = 1.f; // 以 Shell 的密度为基准密度

	switch(deviceType) {
	case IDeviceData::EType::Cabin:
	case IDeviceData::EType::Shell:
	case IDeviceData::EType::Engine:
	case IDeviceData::EType::Joint:
		__NEW_DEF(shellDensity, .5f, 0.f);
		break;
	case IDeviceData::EType::Propeller:
		__NEW_DEF(shellDensity, .5f, 0.f);
		__NEW_DEF(shellDensity * .3f, .5f, .2f);
		break;
	case IDeviceData::EType::JetPropeller:
		__NEW_DEF(shellDensity * .8f, .5f, 0.f);
		break;
	}

#undef __NEW_DEF

	const Frame::Vec2 devicePosMeter = PixelToMeterVec2(devicePos);

#define __NEW_SHAPE(_shape, _b2ShapeType) \
	defs[_ind++].second = { _shape, _b2ShapeType };

	int _ind = 0;

	switch(deviceType) {
	case IDeviceData::EType::Cabin:
	case IDeviceData::EType::Shell:
	case IDeviceData::EType::Engine:
	case IDeviceData::EType::JetPropeller:
	{
		Frame::Vec2 whHalf = GetDeviceMeterSize(deviceType) * .5f;
		b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
		
		__NEW_SHAPE(shape, b2ShapeType::b2_polygonShape);
	}
	break;
	case IDeviceData::EType::Propeller:
	{
		const Frame::Vec2 center1 = Frame::Vec2 { -22.f, 0.f }.GetRotated(rotation) + devicePos;
		const Frame::Vec2 center2 = Frame::Vec2 { 24.f, 0.f }.GetRotated(rotation) + devicePos;
		b2Polygon shape1 = b2MakeOffsetBox(F(24.f), F(40.f), { F(center1.x), F(center1.y) }, b2MakeRot(rotation));
		b2Polygon shape2 = b2MakeOffsetBox(F(36.f), F(114.f), { F(center2.x), F(center2.y) }, b2MakeRot(rotation));

		__NEW_SHAPE(shape1, b2ShapeType::b2_polygonShape);
		__NEW_SHAPE(shape2, b2ShapeType::b2_polygonShape);
	}
	break;
	case IDeviceData::EType::Joint:
	{
		b2Circle shape { { devicePosMeter.x, devicePosMeter.y }, F(36.f) };

		__NEW_SHAPE(shape, b2ShapeType::b2_circleShape);
	}
	break;
	}

#undef __NEW_SHAPE

	return defs;
}

#undef F

void CDeviceComponent::Step(float timeStep, void * userdata) {
	if(!m_pNode || !m_pNode->pDeviceData || !m_pMachinePartEntity) {
		return;
	}
	const IDeviceData::EType deviceType = m_pNode->pDeviceData->device;

	switch(deviceType) {
	case IDeviceData::Propeller:
		if(auto pRigidbodyComp = m_pMachinePartEntity->GetComponent<CRigidbodyComponent>()) {
			const float power = * reinterpret_cast<float *>(userdata);
			pRigidbodyComp->ApplyForce(
				Frame::Vec2 { -1200.f * power, 0.f }.GetRotated(m_pEntity->GetRotation())
				, m_relativePosition.GetRotated(m_pMachinePartEntity->GetRotation()) + m_pMachinePartEntity->GetPosition()
			);
		}
		break;
	}

	timeStep;
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