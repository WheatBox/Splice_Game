#include "EditorDevices.h"

#include "../Assets.h"
#include "../Utility.h"
#include "../Components/ColliderComponent.h"

using E = Assets::EDeviceStaticSprite;

void __Draw(E spr, const Frame::Vec2 & pos, float alpha, float scale, float rot, Frame::ColorRGB color = 0xFFFFFF) {
	Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(spr)->GetImage(), pos, color, alpha, scale, rot);
}

void __AddCollider(CColliderComponent * outColliderComp, Frame::Vec2 size, Frame::Vec2 offset, float rot) {
	size.Rotate(rot);
	size = { std::abs(size.x), std::abs(size.y) };
	offset.Rotate(rot);
	outColliderComp->AddCollider({ size, offset });
}

REGISTER_EDITOR_DEVICE(SCabinEditorDeviceData);

void SCabinEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::cabin_color1, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::cabin_color2, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::cabin, pos, alpha, scale, rot);
}

void SCabinEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, GetEditorDeviceConfig<SCabinEditorDeviceData>().size, { 0.f, 0.f }, rot);
}

void SCabinEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	GetDeviceData<SCabinDeviceData>()->InitSprite(pSpriteComponent, outLayerColors);
}

REGISTER_EDITOR_DEVICE(SShellEditorDeviceData);

void SShellEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::shell_color1, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::shell_color2, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::shell, pos, alpha, scale, rot);
}

void SShellEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, GetEditorDeviceConfig<SShellEditorDeviceData>().size, { 0.f, 0.f }, rot);
}

void SShellEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	GetDeviceData<SShellDeviceData>()->InitSprite(pSpriteComponent, outLayerColors);
}

REGISTER_EDITOR_DEVICE(SEngineEditorDeviceData);

void SEngineEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::engine_color1, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::engine_color2, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::engine, pos, alpha, scale, rot);
}

void SEngineEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, GetEditorDeviceConfig<SEngineEditorDeviceData>().size, { 0.f, 0.f }, rot);
}

void SEngineEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	GetDeviceData<SEngineDeviceData>()->InitSprite(pSpriteComponent, outLayerColors);
}

REGISTER_EDITOR_DEVICE(SPropellerEditorDeviceData);

void SPropellerEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::propeller_motor_color1, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::propeller_motor_color2, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::propeller_motor, pos, alpha, scale, rot);
	DrawSpriteBlendedPro(Assets::GetStaticSprite(E::propeller_blade_color)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.GetRotated(rot) * scale, colorSet.color2, alpha, 30.f, Frame::Vec2 { .3f * scale, scale }, rot);
	DrawSpriteBlendedPro(Assets::GetStaticSprite(E::propeller_blade)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.GetRotated(rot) * scale, 0xFFFFFF, alpha, 30.f, Frame::Vec2 { .3f * scale, scale }, rot);
	__Draw(E::propeller_top_color, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::propeller_top, pos, alpha, scale, rot);
}

void SPropellerEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, { 64.f, 96.f }, { -16.f, 0.f }, rot);
	__AddCollider(outColliderComp, { 72.f, 228.f }, { 24.f, 0.f }, rot);
}

void SPropellerEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	GetDeviceData<SPropellerDeviceData>()->InitSprite(pSpriteComponent, outLayerColors);
}

REGISTER_EDITOR_DEVICE(SJetPropellerEditorDeviceData);

void SJetPropellerEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::jet_propeller_bottom, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::jet_propeller_color1, pos, alpha, scale, rot, colorSet.color1);
	__Draw(E::jet_propeller_color2, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::jet_propeller, pos, alpha, scale, rot);
	__Draw(E::jet_propeller_needle, pos + Frame::Vec2 { 32.f, -20.f }.GetRotated(rot) * scale, alpha, scale, rot + Frame::DegToRad(45.f));
}

void SJetPropellerEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, GetEditorDeviceConfig<SJetPropellerEditorDeviceData>().size, { 0.f, 0.f }, rot);
}

void SJetPropellerEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	GetDeviceData<SJetPropellerDeviceData>()->InitSprite(pSpriteComponent, outLayerColors);
}

REGISTER_EDITOR_DEVICE(SJointEditorDeviceData);

void SJointEditorDeviceData::DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const {
	__Draw(E::joint_bottom, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::joint_color, pos, alpha, scale, rot + Frame::DegToRad(180.f), colorSet.color1);
	__Draw(E::joint, pos, alpha, scale, rot + Frame::DegToRad(180.f));
	__Draw(E::joint_top_color, pos, alpha, scale, rot, colorSet.color2);
	__Draw(E::joint_top, pos, alpha, scale, rot);
}

void SJointEditorDeviceData::InitCollider(CColliderComponent * outColliderComp, float rot) {
	__AddCollider(outColliderComp, GetEditorDeviceConfig<SJointEditorDeviceData>().size, { 0.f, 0.f }, rot);
}

void SJointEditorDeviceData::InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) {
	pSpriteComponent, outLayerColors;
}
