#include "EditorDeviceComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../../Assets.h"
#include "../../Depths.h"

REGISTER_ENTITY_COMPONENT(, CEditorDeviceComponent);

#define __DEBUG_COLLIDER 0

Frame::EntityEvent::Flags CEditorDeviceComponent::GetEventFlags() const {
	return Frame::EntityEvent::Update | Frame::EntityEvent::Render;
}

void CEditorDeviceComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {

	if(!m_pEditorComponent || !m_pEditorComponent->GetWorking()) {
		if(m_pSpriteComponent) {
			m_pSpriteComponent->working = false;
		}
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::Update:
	{
		/*
		const float frametime = event.params[0].f;
		
		if(m_pNode && m_pNode->pDeviceData && m_pNode->pDeviceData->device == IDeviceData::Propeller) {
			//m_pEntity->SetRotation(m_pEntity->GetRotation() + frametime * 30.f);

			const float rot = m_pSpriteComponent->layers[3].GetRotation() + 720.f * frametime;
			m_pSpriteComponent->layers[3].SetRotation(rot);
			m_pSpriteComponent->layers[4].SetRotation(rot);
		}
		*/
	}
	break;
	case Frame::EntityEvent::Render:
	{
		//char sz[5] { 48 + (m_pNode->nodes[0] != nullptr), 48 + (m_pNode->nodes[1] != nullptr), 48 + (m_pNode->nodes[2] != nullptr), 48 + (m_pNode->nodes[3] != nullptr), 0 };
		//Frame::gRenderer->pTextRenderer->DrawTextBlended(sz, m_pEntity->GetPosition() + Frame::Vec2 { 16.f, 48.f }, 0x000000, 1.f);
		
		//Frame::gRenderer->pTextRenderer->DrawTextBlended(std::to_string(m_pipeNodes.size()), m_pEntity->GetPosition() + Frame::Vec2 { 16.f, 48.f }, 0x000000, 1.f);

#if __DEBUG_COLLIDER
		if(m_pColliderComponent) m_pColliderComponent->Render(0xFFFF00, .5f);
#endif
	}
	break;
	}
}

bool CEditorDeviceComponent::Initialize(CEditorComponent * pComp, IDeviceData::EType type, int dirIndex) {
	m_pEntity->SetZDepth(Depths::EditorDevice);

	m_pEntity->SetRotation(-GetDegreeByDirIndex(dirIndex));
	m_directionIndex = dirIndex;

	m_deviceType = type;
	
	m_pColliderComponent = m_pEntity->CreateComponent<CColliderComponent>();

	GetEditorDeviceColliders(m_pColliderComponent, type, dirIndex);

	if(!m_pColliderComponent->Collide().empty()) {
		return false;
	}
	m_pEditorComponent = pComp;

	m_pSpriteComponent = m_pEntity->CreateComponent<CSpriteComponent>();

#define __ADD_SPRITE_LAYER(__EDeviceStaticSprite) \
m_pSpriteComponent->layers.push_back({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), 0xFFFFFF, 1.f }); \
m_colorUpdatesInSpriteLayers.push_back(nullptr);

#define __ADD_SPRITE_LAYER_EXT(__EDeviceStaticSprite, __colorSetMemberVariable) \
m_pSpriteComponent->layers.push_back({ Assets::GetStaticSprite(Assets::EDeviceStaticSprite::__EDeviceStaticSprite), 0xFFFFFF, 1.f }); \
m_colorUpdatesInSpriteLayers.push_back(& SColorSet::__colorSetMemberVariable);

	if(type == IDeviceData::Cabin) {
		__ADD_SPRITE_LAYER(cabin_logo_background);
	} else if(type == IDeviceData::JetPropeller) {
		__ADD_SPRITE_LAYER_EXT(jet_propeller_bottom, color1);
	}

	m_colorUpdatesInSpriteLayers.push_back(& SColorSet::color1);
	m_colorUpdatesInSpriteLayers.push_back(& SColorSet::color2);
	m_colorUpdatesInSpriteLayers.push_back(nullptr);
#if !__DEBUG_COLLIDER
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::color1), 0xFFFFFF, 1.f });
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::color2), 0xFFFFFF, 1.f });
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::basic), 0xFFFFFF, 1.f });
#else
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::color1), 0xFFFFFF, .5f });
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::color2), 0xFFFFFF, .5f });
	m_pSpriteComponent->layers.push_back({ Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::basic), 0xFFFFFF, .5f });
#endif

	if(type == IDeviceData::Propeller) {
		__ADD_SPRITE_LAYER_EXT(propeller_blade_color, color2);
		{
			auto & layerTemp = m_pSpriteComponent->layers.back();
			layerTemp.SetScale({ .3f, 1.f });
			layerTemp.SetOffset({ -20.f, 0.f });
			layerTemp.SetRotation(30.f);
		}
		__ADD_SPRITE_LAYER(propeller_blade);
		{
			auto & layerTemp = m_pSpriteComponent->layers.back();
			layerTemp.SetScale({ .3f, 1.f });
			layerTemp.SetOffset({ -20.f, 0.f });
			layerTemp.SetRotation(30.f);
		}

		__ADD_SPRITE_LAYER_EXT(propeller_top_color, color1);
		__ADD_SPRITE_LAYER(propeller_top);
	} else if(type == IDeviceData::JetPropeller) {
		__ADD_SPRITE_LAYER(jet_propeller_needle);
		{
			auto & layerTemp = m_pSpriteComponent->layers.back();
			layerTemp.SetOffset({ -32.f, 20.f });
			layerTemp.SetRotation(45.f);
		}
	}

#undef __ADD_SPRITE_LAYER
#undef __ADD_SPRITE_LAYER_EXT

	return true;
}

void CEditorDeviceComponent::OnShutDown() {
	for(int i = 0; i < 4; i++) {
		if(m_neighbors[i] == nullptr) {
			continue;
		}
		m_neighbors[i]->m_neighbors[GetRevDirIndex(i)] = nullptr;
	}
}

void CEditorDeviceComponent::GetAvailableInterfaces(std::vector<CEditorComponent::SInterface> * outToPushBack) {
	if(m_deviceType == IDeviceData::Unset) {
		return;
	}

#define __FORMULA(dirIndex) \
if(m_neighbors[dirIndex] == nullptr) { \
	Frame::Vec2 pos = m_pEntity->GetPosition() + GetDeviceInterfaceBias(m_deviceType, m_directionIndex, dirIndex, 0.f) + GetRectangleEdgePosByDirIndex(GetDevicePixelSize(m_deviceType) + 16.f, m_directionIndex, dirIndex); \
	outToPushBack->push_back({ this, pos, dirIndex }); \
}

	switch(m_deviceType) {
	case IDeviceData::Cabin:
	case IDeviceData::Shell:
	case IDeviceData::Engine:
		for(int i = 0; i < 4; i++) {
			__FORMULA(i)
		}
		break;
	case IDeviceData::Propeller:
		__FORMULA(GetRevDirIndex(m_directionIndex))
		break;
	case IDeviceData::JetPropeller:
		__FORMULA(GetRevDirIndex(m_directionIndex))
		__FORMULA(GetLeftDirIndex(m_directionIndex))
		__FORMULA(GetRightDirIndex(m_directionIndex))
		break;
	}

#undef __FORMULA
}

void CEditorDeviceComponent::GetPipeInterfaces(std::vector<CEditorComponent::SPipeInterface> * outToPushBack) {
	if(m_deviceType == IDeviceData::Unset) {
		return;
	}

#define __FORMULA(dirIndex) { \
	Frame::Vec2 pos = m_pEntity->GetPosition() + GetRectangleEdgePosByDirIndex(GetDevicePixelSize(m_deviceType), m_directionIndex, dirIndex); \
	outToPushBack->push_back({ this, pos, dirIndex }); \
}

	switch(m_deviceType) {
	case IDeviceData::Engine:
		for(int i = 0; i < 4; i++) {
			__FORMULA(i)
		}
		break;
	case IDeviceData::Propeller:
	case IDeviceData::JetPropeller:
		__FORMULA(GetRevDirIndex(m_directionIndex))
		break;
	}

#undef __FORMULA
}

bool CEditorDeviceComponent::DeviceTreeNodeConnectWith(CEditorDeviceComponent * pEDComp, int dirIndex) {
	if(!pEDComp) {
		return false;
	}
	int revDirIndex = GetRevDirIndex(dirIndex);
	if(m_neighbors[dirIndex] || pEDComp->m_neighbors[revDirIndex]) {
		return false;
	}
	m_neighbors[dirIndex] = pEDComp;
	pEDComp->m_neighbors[revDirIndex] = this;
	return true;
}

void CEditorDeviceComponent::DrawConnectors() {
	for(int i = 0; i < 2; i++) {
		if(m_neighbors[i] == nullptr) {
			continue;
		}
		Frame::Vec2 pos = m_pEntity->GetPosition()
			+ GetDeviceInterfaceBias(m_deviceType, m_directionIndex, i, 0.f)
			+ GetRectangleEdgePosByDirIndex(GetDevicePixelSize(m_deviceType) + CONNECTOR_HALF_LENGTH * 2.f, m_directionIndex, i)
			;
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::connector)->GetImage(), pos, m_colorSet.connector, std::min(m_alpha, m_neighbors[i]->GetAlpha()), { 1.f }, i * 90.f);
	}
}

void CEditorDeviceComponent::GetEditorDeviceColliders(CColliderComponent * outColliderComp, IDeviceData::EType type, int dirIndex) {
#define __FORMULA(w, h, offx, offy) \
{ \
	Frame::Vec2 _siz = Frame::Vec2 { w, h }.RotateDegree(-GetDegreeByDirIndex(dirIndex)); \
	outColliderComp->AddCollider({ { std::abs(_siz.x), std::abs(_siz.y) }, Frame::Vec2 { offx, offy }.RotateDegree(-GetDegreeByDirIndex(dirIndex)) }); \
}

	const Frame::Vec2 siz = GetDevicePixelSize(type);

	switch(type) {
	default:
		__FORMULA(siz.x, siz.y, 0.f, 0.f)
		break;
	case IDeviceData::Propeller:
		__FORMULA(64.f, 96.f, -16.f, 0.f)
		__FORMULA(72.f, 228.f, 24.f, 0.f)
		break;
	}

#undef __FORMULA
}
