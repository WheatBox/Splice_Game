#include "DevicesData.h"

#include "Assets.h"
#include "Components/Machine/DeviceComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/SmokeEmitterComponent.h"

std::vector<std::unique_ptr<IDeviceData>> & GetDeviceRegistry() {
	static std::vector<std::unique_ptr<IDeviceData>> registry {};
	return registry;
}

#define __SPRITE_ALPHA 1.f

CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB SColorSet::* col) {
	pSpriteComponent->AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA });
	layerColors.push_back(col);
	return pSpriteComponent->GetLayers().back();
}
CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite) {
	return AddSpriteLayer(pSpriteComponent, layerColors, _EDeviceStaticSprite, nullptr);
}
CSpriteComponent::SLayer & AddSpriteLayer(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB SColorSet::* col, int _CDeviceComponent_InsBufferGroupIndex) {
	pSpriteComponent->AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA }, _CDeviceComponent_InsBufferGroupIndex);
	layerColors.push_back(col);
	return pSpriteComponent->GetLayers().back();
}

#undef __SPRITE_ALPHA

#define ADD_SPRITE_LAYER_1(_EDeviceStaticSprite) AddSpriteLayer(pSpriteComponent, layerColors, _EDeviceStaticSprite)
#define ADD_SPRITE_LAYER_2(_EDeviceStaticSprite, col) AddSpriteLayer(pSpriteComponent, layerColors, _EDeviceStaticSprite, col)
#define ADD_SPRITE_LAYER_3(_EDeviceStaticSprite, col, _CDeviceComponent_InsBufferGroupIndex) AddSpriteLayer(pSpriteComponent, layerColors, _EDeviceStaticSprite, col, _CDeviceComponent_InsBufferGroupIndex)

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

template<typename DataT>
std::pair<b2ShapeDef, SBox2dShape> MakeShapeDefDefaultShell(const Frame::Vec2 & devicePos, float rotation) {
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = GetDeviceConfig<DataT>().size * .5f;
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	return MakeShapeDef(1.f, .5f, 0.f, shape);
}

using E = Assets::EDeviceStaticSprite;
using C = SColorSet;

constexpr int staticGroup = CDeviceComponent::staticInsBufferGroupIndex;
constexpr int dynamicGroup = CDeviceComponent::dynamicInsBufferGroupIndex;
constexpr int staticTopGroup = CDeviceComponent::staticTopInsBufferGroupIndex;

REGISTER_DEVICE(SCabinDeviceData)

void SCabinDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) {
	ADD_SPRITE_LAYER_1(E::cabin_logo_background);
	ADD_SPRITE_LAYER_2(E::cabin_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::cabin_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::cabin);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SCabinDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell<SCabinDeviceData>(devicePos, rotation) };
}

REGISTER_DEVICE(SShellDeviceData)

void SShellDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) {
	ADD_SPRITE_LAYER_2(E::shell_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::shell_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::shell);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SShellDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell<SShellDeviceData>(devicePos, rotation) };
}

REGISTER_DEVICE(SEngineDeviceData)

void SEngineDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) {
	ADD_SPRITE_LAYER_2(E::engine_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::engine_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::engine);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SEngineDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefaultShell<SEngineDeviceData>(devicePos, rotation) };
}

REGISTER_DEVICE(SPropellerDeviceData)

void SPropellerDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) {
	ADD_SPRITE_LAYER_2(E::propeller_motor_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::propeller_motor_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::propeller_motor);

	ADD_SPRITE_LAYER_3(E::propeller_blade_color, & C::color2, dynamicGroup)
		.SetScale({ .3f, 1.f })
		.SetOffset({ 20.f, 0.f })
		.SetRotationDegree(30.f);
	ADD_SPRITE_LAYER_3(E::propeller_blade, nullptr, dynamicGroup)
		.SetScale({ .3f, 1.f })
		.SetOffset({ 20.f, 0.f })
		.SetRotationDegree(30.f);

	ADD_SPRITE_LAYER_3(E::propeller_top_color, & C::color1, staticTopGroup);
	ADD_SPRITE_LAYER_3(E::propeller_top, nullptr, staticTopGroup);
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

REGISTER_DEVICE(SJetPropellerDeviceData)

void SJetPropellerDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) {
	if(CDeviceComponent * pDeviceComponent = pSpriteComponent->GetEntity()->GetComponent<CDeviceComponent>(); !pDeviceComponent) {
		ADD_SPRITE_LAYER_2(E::jet_propeller_bottom, & C::color1);
	} else
	ADD_SPRITE_LAYER_2(E::jet_propeller_bottom, & C::color1)
		/*.SetExtraFunction( // TODO
			[pDeviceComponent](float frameTime) {
				auto pNode = pDeviceComponent->GetNode();
				if(!pNode || !pNode->pDeviceData) {
					return;
				}

				auto pEntity = pDeviceComponent->GetEntity();
				SJetPropellerDeviceData * pData = reinterpret_cast<SJetPropellerDeviceData *>(pNode->pDeviceData);

				const Frame::Vec2 entPos = pEntity->GetPosition();
				const float entRot = pEntity->GetRotation();

				const Frame::Vec2 pos = entPos - Frame::Vec2 { 32.f, 0.f }.GetRotated(entRot);
				const float alpha = Frame::Min(1.f, pData->accumulatingShowing / pData->accumulationShowingMax) * .35f;

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
		)*/;
	ADD_SPRITE_LAYER_2(E::jet_propeller_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::jet_propeller_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::jet_propeller);
	ADD_SPRITE_LAYER_1(E::jet_propeller_needle)
		.SetOffset({ 32.f, -20.f })
		.SetRotationDegree(45.f);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJetPropellerDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = GetDeviceConfig<SJetPropellerDeviceData>().size * .5f;
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(.8f, .5f, 0.f, shape));
	return defs;
}

//void SJointDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, CDeviceComponent *, const SColorSet & colorSet) {
//	AddSpriteLayer(pSpriteComponent, E::joint_bottom, colorSet.color2);
//	AddSpriteLayer_Part(pSpriteComponent, device, EP::color1, colorSet.color1)
//		.SetRotationDegree(180.f);
//	AddSpriteLayer_Part(pSpriteComponent, device, EP::basic)
//		.SetRotationDegree(180.f);
//	AddSpriteLayer(pSpriteComponent, E::joint_top_color, colorSet.color2);
//	AddSpriteLayer(pSpriteComponent, E::joint_top);
//}
//
//std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJointDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float) {
//	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
//	const Frame::Vec2 devicePosMeter = P2M(devicePos);
//	const b2Circle shape { { devicePosMeter.x, devicePosMeter.y }, P2M(36.f) };
//	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape));
//	return defs;
//}