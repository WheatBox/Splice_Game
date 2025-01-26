#include "DevicesData.h"

#include "Assets.h"
#include "Components/Machine/DeviceComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/SmokeEmitterComponent.h"

CDeviceComponent * SDeviceDataMachinePartJoint::GetPointMachinePartDeviceComponent() const {
	if(m_pPointMachinePartDeviceComponent && CDeviceComponent::s_workingDevices.find(m_pPointMachinePartDeviceComponent) != CDeviceComponent::s_workingDevices.end()) {
		return m_pPointMachinePartDeviceComponent;
	}
	return nullptr;
}

CDeviceComponent * SDeviceDataMachinePartJoint::GetBehindMachinePartDeviceComponent() const {
	if(m_pBehindMachinePartDeviceComponent && CDeviceComponent::s_workingDevices.find(m_pBehindMachinePartDeviceComponent) != CDeviceComponent::s_workingDevices.end()) {
		return m_pBehindMachinePartDeviceComponent;
	}
	return nullptr;
}

#define __SPRITE_ALPHA 1.f

CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB col) {
	pSpriteComponent->AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), col, __SPRITE_ALPHA });
	return pSpriteComponent->GetLayers().back();
}
CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, Assets::EDeviceStaticSprite _EDeviceStaticSprite) {
	return AddSpriteLayer(pSpriteComponent, _EDeviceStaticSprite, 0xFFFFFF);
}
CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB col, int _CDeviceComponent_InsBufferGroupIndex) {
	pSpriteComponent->AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), col, __SPRITE_ALPHA }, _CDeviceComponent_InsBufferGroupIndex);
	return pSpriteComponent->GetLayers().back();
}

CSpriteComponent::SLayer & AddSpriteLayer_Part(CSpriteComponent * pSpriteComponent, IDeviceData::EType type, Assets::EDeviceStaticSpritePart _EDeviceStaticSpritePart, Frame::ColorRGB col) {
	pSpriteComponent->AddLayer({ Assets::GetDeviceStaticSprite(type, _EDeviceStaticSpritePart), col, __SPRITE_ALPHA });
	return pSpriteComponent->GetLayers().back();
}
CSpriteComponent::SLayer & AddSpriteLayer_Part(CSpriteComponent * pSpriteComponent, IDeviceData::EType type, Assets::EDeviceStaticSpritePart _EDeviceStaticSpritePart) {
	return AddSpriteLayer_Part(pSpriteComponent, type, _EDeviceStaticSpritePart, 0xFFFFFF);
}
CSpriteComponent::SLayer & AddSpriteLayer_Part(CSpriteComponent * pSpriteComponent, IDeviceData::EType type, Assets::EDeviceStaticSpritePart _EDeviceStaticSpritePart, Frame::ColorRGB col, int _CDeviceComponent_InsBufferGroupIndex) {
	pSpriteComponent->AddLayer({ Assets::GetDeviceStaticSprite(type, _EDeviceStaticSpritePart), col, __SPRITE_ALPHA }, _CDeviceComponent_InsBufferGroupIndex);
	return pSpriteComponent->GetLayers().back();
}

#undef __SPRITE_ALPHA

float P2M(float pixel) {
	return PixelToMeter(pixel);
}
Frame::Vec2 P2M(const Frame::Vec2 & pixel) {
	return PixelToMeterVec2(pixel);
}

b2ShapeDef __MakeShapeDef(float density, float friction, float restitution) {
	b2ShapeDef defTemp = b2DefaultShapeDef();
	defTemp.density = density;
	defTemp.friction = friction;
	defTemp.restitution = restitution;
	return defTemp;
}

std::pair<b2ShapeDef, SBox2dShape> MakeShapeDef(float density, float friction, float restitution, const b2Polygon & polygon) {
	return { __MakeShapeDef(density, friction, restitution), { polygon, b2ShapeType::b2_polygonShape } };
}
std::pair<b2ShapeDef, SBox2dShape> MakeShapeDef(float density, float friction, float restitution, const b2Circle & circle) {
	return { __MakeShapeDef(density, friction, restitution), { circle, b2ShapeType::b2_circleShape } };
}

std::pair<b2ShapeDef, SBox2dShape> MakeShapeDefDefaultShell(IDeviceData::EType device, const Frame::Vec2 & devicePos, float rotation) {
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = GetDeviceMeterSize(device) * .5f;
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	return MakeShapeDef(1.f, .5f, 0.f, shape);
}

using E = Assets::EDeviceStaticSprite;
using EP = Assets::EDeviceStaticSpritePart;

constexpr int staticGroup = CDeviceComponent::staticInsBufferGroupIndex;
constexpr int dynamicGroup = CDeviceComponent::dynamicInsBufferGroupIndex;
constexpr int staticTopGroup = CDeviceComponent::staticTopInsBufferGroupIndex;

void SCabinDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
	AddSpriteLayer(pSpriteComponent, E::cabin_logo_background);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color2, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SCabinDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell(device, devicePos, rotation) };
}

void SShellDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color2, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SShellDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell(device, devicePos, rotation) };
}

void SEngineDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color2, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SEngineDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell(device, devicePos, rotation) };
}

void SPropellerDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color2, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic);

	AddSpriteLayer(pSpriteComponent, E::propeller_blade_color, colorSet.color2, dynamicGroup)
		.SetScale({ .3f, 1.f })
		.SetOffset({ -20.f, 0.f })
		.SetRotationDegree(30.f);
	AddSpriteLayer(pSpriteComponent, E::propeller_blade, 0xFFFFFF, dynamicGroup)
		.SetScale({ .3f, 1.f })
		.SetOffset({ -20.f, 0.f })
		.SetRotationDegree(30.f);

	AddSpriteLayer(pSpriteComponent, E::propeller_top_color, colorSet.color1, staticTopGroup);
	AddSpriteLayer(pSpriteComponent, E::propeller_top, 0xFFFFFF, staticTopGroup);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SPropellerDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 center1 = Frame::Vec2 { -22.f, 0.f }.GetRotated(rotation) + devicePos;
	const Frame::Vec2 center2 = Frame::Vec2 { 24.f, 0.f }.GetRotated(rotation) + devicePos;
	const b2Polygon shape1 = b2MakeOffsetBox(P2M(24.f), P2M(40.f), { P2M(center1.x), P2M(center1.y) }, b2MakeRot(rotation));
	const b2Polygon shape2 = b2MakeOffsetBox(P2M(36.f), P2M(114.f), { P2M(center2.x), P2M(center2.y) }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape1));
	defs.push_back(MakeShapeDef(.3f, .5f, .2f, shape2));
	return defs;
}

void SJetPropellerDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent * pDeviceComponent, const SColorSet & colorSet) {
	AddSpriteLayer(pSpriteComponent, E::jet_propeller_bottom, colorSet.color1)
		.SetExtraFunction(
			[pDeviceComponent](float frameTime) {
				auto pNode = pDeviceComponent->GetNode();
				if(!pNode || !pNode->pDeviceData) {
					return;
				}

				auto pEntity = pDeviceComponent->GetEntity();
				auto pData = reinterpret_cast<SJetPropellerDeviceData *>(pNode->pDeviceData);

				const Frame::Vec2 entPos = pEntity->GetPosition();
				const float entRot = pEntity->GetRotation();

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
			}
		);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color2, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic);
	AddSpriteLayer(pSpriteComponent, E::jet_propeller_needle)
		.SetOffset({ -32.f, 20.f })
		.SetRotationDegree(45.f);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJetPropellerDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = GetDeviceMeterSize(device) * .5f;
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(.8f, .5f, 0.f, shape));
	return defs;
}

void SJointDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
	AddSpriteLayer(pSpriteComponent, E::joint_bottom, colorSet.color2);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1)
		.SetRotationDegree(180.f);
	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic)
		.SetRotationDegree(180.f);
	AddSpriteLayer(pSpriteComponent, E::joint_top_color, colorSet.color2);
	AddSpriteLayer(pSpriteComponent, E::joint_top);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJointDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const b2Circle shape { { devicePosMeter.x, devicePosMeter.y }, P2M(36.f) };
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape));
	return defs;
}
