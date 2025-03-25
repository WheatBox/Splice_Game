#include "Devices.h"

#include "Utils.h"
#include "../Utility.h"

#include "../Components/SmokeEmitterComponent.h"
#include "../Components/Machine/MachinePartComponent.h"

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
std::pair<b2ShapeDef, SBox2dShape> MakeShapeDefDefault(const Frame::Vec2 & devicePos, float rotation) {
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = P2M(48.f /*96.f * .5f*/);
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	return MakeShapeDef(1.f, .5f, 0.f, shape);
}

REGISTER_DEVICE(SCabinDeviceData);

void SCabinDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	ADD_SPRITE_LAYER_1(E::cabin_logo_background);
	ADD_SPRITE_LAYER_2(E::cabin_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::cabin_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::cabin);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SCabinDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefault<SCabinDeviceData>(devicePos, rotation) };
}

const std::map<int, SDeviceInterfaceDef> & SCabinDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs(96.f, { 0, 1, 2, 3 }, { 0, 90, 180, 270 });
	return map;
}

REGISTER_DEVICE(SShellDeviceData);

void SShellDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	ADD_SPRITE_LAYER_2(E::shell_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::shell_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::shell);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SShellDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefault<SShellDeviceData>(devicePos, rotation) };
}

const std::map<int, SDeviceInterfaceDef> & SShellDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs(96.f, { 0, 1, 2, 3 }, { 0, 90, 180, 270 });
	return map;
}

REGISTER_DEVICE(SEngineDeviceData);

void SEngineDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	ADD_SPRITE_LAYER_2(E::engine_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::engine_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::engine);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SEngineDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	return { MakeShapeDefDefault<SEngineDeviceData>(devicePos, rotation) };
}

const std::map<int, SDeviceInterfaceDef> & SEngineDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs(96.f, { 0, 1, 2, 3 }, { 0, 90, 180, 270 });
	return map;
}

REGISTER_DEVICE(SPropellerDeviceData);

void SPropellerDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	ADD_SPRITE_LAYER_2(E::propeller_motor_color1, & C::color1);
	ADD_SPRITE_LAYER_2(E::propeller_motor_color2, & C::color2);
	ADD_SPRITE_LAYER_1(E::propeller_motor);

	ADD_SPRITE_LAYER_3(E::propeller_blade_color, & C::color2, eDSG_Dynamic)
		.SetScale({ .3f, 1.f })
		.SetOffset({ 20.f, 0.f })
		.SetRotationDegree(30.f);
	ADD_SPRITE_LAYER_3(E::propeller_blade, nullptr, eDSG_Dynamic)
		.SetScale({ .3f, 1.f })
		.SetOffset({ 20.f, 0.f })
		.SetRotationDegree(30.f);

	bladeLayerIndex = sprite.GetLayers().size() - 1;
	bladeColorLayerIndex = bladeLayerIndex - 1;

	ADD_SPRITE_LAYER_3(E::propeller_top_color, & C::color1, eDSG_StaticTop);
	ADD_SPRITE_LAYER_3(E::propeller_top, nullptr, eDSG_StaticTop);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SPropellerDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 center1 = Frame::Vec2 { -22.f, 0.f }.GetRotated(rotation) + devicePos;
	const b2Polygon shape1 = b2MakeOffsetBox(P2M(24.f), P2M(40.f), { P2M(center1.x), P2M(center1.y) }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape1));

	//const Frame::Vec2 center2 = P2M(Frame::Vec2 { 24.f, 0.f }.GetRotated(rotation) + devicePos);
	//const b2Polygon shape2 = b2MakeOffsetBox(P2M(36.f), P2M(114.f), { P2M(center2.x), P2M(center2.y) }, b2MakeRot(rotation));
	//defs.push_back(MakeShapeDef(.3f, .5f, .2f, shape2));

	const Frame::Vec2 center2InMeter = P2M(Frame::Vec2 { 20.f, 0.f }.GetRotated(rotation) + devicePos);
	b2Vec2 points[8];
	for(int i = 0; i < 8; i++) {
		float angle = Frame::pi_f * .25f * i;
		Frame::Vec2 point = { P2M(44.f * cosf(angle)), P2M(120.f * sinf(angle)) };
		point.Rotate(rotation);
		points[i] = { point.x + center2InMeter.x, point.y + center2InMeter.y };
	}
	b2Hull hull = b2ComputeHull(points, 8);
	const b2Polygon shape2 = b2MakePolygon(& hull, 0.f);
	defs.push_back(MakeShapeDef(.3f, .5f, .2f, shape2));
	return defs;
}

const std::map<int, SDeviceInterfaceDef> & SPropellerDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs({ 96.f, 240.f }, { 0 }, { 180 });
	return map;
}

float SPropellerDeviceData::PreStep(const SPreStepParams & params) {
	const float dotval = params.facingDir.Dot(params.machinePartTargetMovingDir);
	return dotval >= 0.f ? 0.f : -dotval;
}

void SPropellerDeviceData::Step(const SStepParams & params) {

	CRigidbodyComponent * pRigidbody = m_pBelongingMachinePart->GetRigidbody();
	if(!pRigidbody) {
		return;
	}

	const float deviceRot = GetRotation();
	const Frame::Vec2 devicePos = GetPosition();

	pRigidbody->ApplyForce(
		Frame::Vec2 { -1200.f * params.power, 0.f }.GetRotated(deviceRot)
		, devicePos
	);

	if(auto & layers = m_sprite.GetLayers(); layers.size() > bladeLayerIndex) {
		const float rot = layers[bladeLayerIndex].GetRotationDegree() + (1000.f + 600.f * params.power) * params.timeStep;
		layers[bladeColorLayerIndex].SetRotationDegree(rot);
		layers[bladeLayerIndex].SetRotationDegree(rot);
	}
}

REGISTER_DEVICE(SJetPropellerDeviceData);

void SJetPropellerDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	smokeRotation1 = Frame::DegToRad(static_cast<float>(rand() % 360));
	smokeRotation2 = Frame::DegToRad(static_cast<float>(rand() % 360));

	ADD_SPRITE_LAYER_2(E::jet_propeller_bottom, & C::color1);

	smokeLayerIndexBegin = static_cast<int>(sprite.GetLayers().size());
	constexpr E smokes[] = { E::jet_propeller_smoke1, E::jet_propeller_smoke2, E::jet_propeller_smoke3, E::jet_propeller_smoke4, E::jet_propeller_smoke5 };
	ADD_SPRITE_LAYER_3(smokes[rand() % 5], nullptr, eDSG_Dynamic).SetScale({ .75f }).SetAlpha(0.f);
	ADD_SPRITE_LAYER_3(smokes[rand() % 5], nullptr, eDSG_Dynamic).SetScale({ .75f }).SetAlpha(0.f);
	ADD_SPRITE_LAYER_3(smokes[rand() % 5], nullptr, eDSG_Dynamic).SetScale({ .75f }).SetAlpha(0.f);
	ADD_SPRITE_LAYER_3(smokes[rand() % 5], nullptr, eDSG_Dynamic).SetScale({ .75f }).SetAlpha(0.f);
	ADD_SPRITE_LAYER_3(smokes[rand() % 5], nullptr, eDSG_Dynamic).SetScale({ .75f }).SetAlpha(0.f);
	smokeLayerIndexEnd = static_cast<int>(sprite.GetLayers().size());

	ADD_SPRITE_LAYER_3(E::jet_propeller_color1, & C::color1, eDSG_StaticTop);
	ADD_SPRITE_LAYER_3(E::jet_propeller_color2, & C::color2, eDSG_StaticTop);
	ADD_SPRITE_LAYER_3(E::jet_propeller, nullptr, eDSG_StaticTop);
	ADD_SPRITE_LAYER_3(E::jet_propeller_needle, nullptr, eDSG_DynamicTop)
		.SetOffset({ 32.f, -20.f })
		.SetRotationDegree(45.f);
	needleLayerIndex = static_cast<int>(sprite.GetLayers().size() - 1);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJetPropellerDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const Frame::Vec2 whHalf = P2M(Frame::Vec2 { 184.f, 96.f } * .5f);
	const b2Polygon shape = b2MakeOffsetBox(whHalf.x, whHalf.y, { devicePosMeter.x, devicePosMeter.y }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(.8f, .5f, 0.f, shape));
	return defs;
}

const std::map<int, SDeviceInterfaceDef> & SJetPropellerDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs({ 184.f, 96.f }, { 0, 1, 2 }, { 180, 90, 270 });
	static bool inited = false;
	if(!inited) {
		inited = true;
		map[1].offset += { -44.f, 0.f };
		map[2].offset += { -44.f, 0.f };
	}
	return map;
}

float SJetPropellerDeviceData::PreStep(const SPreStepParams & params) {
	const float dotval = params.facingDir.Dot(params.machinePartTargetMovingDir);
	return dotval >= 0.f ? 0.f : -dotval;
}

void SJetPropellerDeviceData::Step(const SStepParams & params) {
	CRigidbodyComponent * pRigidbody = m_pBelongingMachinePart->GetRigidbody();
	if(!pRigidbody) {
		return;
	}

	const float rot = GetRotation();
	const Frame::Vec2 pos = GetPosition();

	if(accumulating < 0.f) {
		accumulating += params.timeStep * 1.f;
		if(accumulating > 0.f) {
			accumulating = 0.f;
		}
	} else {
		accumulating += params.timeStep * params.power;
	}
	if(accumulating >= accumulationMax) {
		pRigidbody->ApplyLinearImpulse(
			Frame::Vec2 { -6000.f, 0.f }.GetRotated(rot)
			, pos
		);
		accumulating = -.5f;
	}
	accumulatingShowingPrev = accumulatingShowing;
	accumulatingShowing = Lerp(accumulatingShowing, Frame::Max(accumulating, 0.f), params.timeStep * 10.f);

	const float alpha = Frame::Min(1.f, accumulatingShowing / accumulationShowingMax) * .35f;
	smokeRotation1 += params.timeStep * (40.f + 30000.f * (accumulatingShowing - accumulatingShowingPrev));
	smokeRotation2 += params.timeStep * (80.f + 60000.f * (accumulatingShowing - accumulatingShowingPrev));
	for(int i = smokeLayerIndexBegin; i < smokeLayerIndexEnd; i++) {
		auto & layer = m_sprite.GetLayers()[i];
		layer.SetOffset(Frame::Vec2 { -32.f, 0 } - Frame::Vec2 { 16.f, 0.f }.GetRotatedDegree(static_cast<float>((i - smokeLayerIndexBegin) * (360 / (smokeLayerIndexEnd - smokeLayerIndexBegin))) + smokeRotation1));
		layer.SetRotationDegree(smokeRotation2);
		layer.SetAlpha(alpha);
	}

	{
		const float needleRot = Frame::DegToRad(45.f)
			+ Frame::DegToRad(270.f) * Frame::Clamp(accumulatingShowing, 0.f, accumulationShowingMax) / accumulationShowingMax
			+ (accumulatingShowing <= accumulationShowingMax ? 0.f : Frame::DegToRad(12.f) * -std::sin(30.f * (accumulationShowingMax - accumulatingShowing)))
			;
		m_sprite.GetLayers()[needleLayerIndex].SetRotation(needleRot);
	}

	if(accumulatingShowing > .002f && accumulating < 0.f) {
		for(int i = 0; i < 4; i++) {
			CSmokeEmitterComponent::SSmokeParticle particle {
				pos + Frame::Vec2 { 96.f, 0.f }.GetRotated(rot),
				1.f, 0xFFFFFF, Frame::Vec2 { 4000.f, 0.f }.GetRotated(rot + Frame::DegToRad(static_cast<float>(rand() % 41 - 20)))
			};
			particle.alpha *= -accumulating * 3.f;
			particle.scaleAdd *= 5.f;
			particle.alphaAdd *= 1.5f;
			CSmokeEmitterComponent::SummonSmokeParticle(particle);
		}
	}
}

REGISTER_DEVICE(SJointRootDeviceData);

void SJointRootDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet:: *> & outLayerColors) {
	ADD_SPRITE_LAYER_3(E::joint_color, & C::color1, eDSG_Static).SetRotationDegree(180.f);
	ADD_SPRITE_LAYER_3(E::joint, nullptr, eDSG_Static).SetRotationDegree(180.f);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJointRootDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	
	const Frame::Vec2 devicePosMeter = P2M(devicePos);
	const b2Circle shape { { devicePosMeter.x, devicePosMeter.y }, P2M(36.f) };
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape));

	const Frame::Vec2 center1 = Frame::Vec2 { -46.f, 0.f }.GetRotated(rotation) + devicePos;
	const b2Polygon shape1 = b2MakeOffsetBox(P2M(4.f), P2M(36.f), { P2M(center1.x), P2M(center1.y) }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape1));

	return defs;
}

const std::map<int, SDeviceInterfaceDef> & SJointRootDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs(96.f, {1}, {180});
	return map;
}

void SJointRootDeviceData::InitJoint(const std::vector<std::shared_ptr<IDeviceData>> & devices) {
	for(auto & pDevice : devices) {
		if(pDevice.get() == this) {
			continue;
		}
		Frame::Vec2 _worldPivot = P2M(GetPosition());
		b2Vec2 worldPivot { _worldPivot.x, _worldPivot.y };
		b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
		jointDef.bodyIdA = m_pBelongingMachinePart->GetRigidbody()->GetBody();
		jointDef.bodyIdB = pDevice->m_pBelongingMachinePart->GetRigidbody()->GetBody();
		jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, worldPivot);
		jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, worldPivot);

		jointDef.collideConnected = true;

		b2JointId myJointId = b2CreateRevoluteJoint(gWorldId, & jointDef);
	}
}

REGISTER_DEVICE(SJointSecondDeviceData);

void SJointSecondDeviceData::InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet:: *> & outLayerColors) {
	ADD_SPRITE_LAYER_3(E::joint_bottom, & C::color2, eDSG_StaticBottom);
	ADD_SPRITE_LAYER_3(E::joint_top_color, & C::color2, eDSG_StaticTop);
	ADD_SPRITE_LAYER_3(E::joint_top, nullptr, eDSG_StaticTop);
}

std::vector<std::pair<b2ShapeDef, SBox2dShape>> SJointSecondDeviceData::MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) {
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> defs;
	const Frame::Vec2 center1 = Frame::Vec2 { 46.f, 0.f }.GetRotated(rotation) + devicePos;
	const b2Polygon shape1 = b2MakeOffsetBox(P2M(4.f), P2M(36.f), { P2M(center1.x), P2M(center1.y) }, b2MakeRot(rotation));
	defs.push_back(MakeShapeDef(1.f, .5f, 0.f, shape1));
	return defs;
}

const std::map<int, SDeviceInterfaceDef> & SJointSecondDeviceData::GetInterfaceDefs() {
	static std::map<int, SDeviceInterfaceDef> map = EasyMakeDeviceInterfaceDefs(96.f, {0}, {0});
	return map;
}
