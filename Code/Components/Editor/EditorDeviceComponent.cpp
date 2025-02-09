#include "EditorDeviceComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../../Assets.h"
#include "../../Depths.h"
#include "EditorComponent.h"
#include "FrameAsset/Sprite.h"
#include "FrameMath/Matrix33.h"

REGISTER_ENTITY_COMPONENT(CEditorDeviceComponent);

Frame::EntityEvent::Flags CEditorDeviceComponent::GetEventFlags() const {
	return Frame::EntityEvent::Update | Frame::EntityEvent::Render;
}

void CEditorDeviceComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {

	if(!m_bWorking) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::Update:
	{
		/*
		const float frametime = event.params[0].f;
		
		if(m_pNode && m_pNode->pDeviceData && m_pNode->pDeviceData->device == IDeviceData::Propeller) {
			//m_pEntity->SetRotation(m_pEntity->GetRotation() + Frame::DegToRad(frametime * 30.f));

			const float rot = m_pSpriteComponent->layers[3].GetRotation() + Frame::DegToRad(720.f * frametime);
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
		
#if 0
		if(m_pSpriteComponent) for(auto & layer : m_pSpriteComponent->GetLayers()) { layer.SetAlpha(.3f); }
		if(m_pColliderComponent) {
			m_pColliderComponent->Render(0x0000FF, .7f);
		}
#endif
	}
	break;
	}
}

bool CEditorDeviceComponent::Initialize(const Frame::GUID & editorDeviceGUID, float rotation) {
	m_pEntity->SetZDepth(Depths::EditorDevice);

	m_pEntity->SetRotation(rotation);

	m_pData = GetEditorDeviceData(editorDeviceGUID)->NewShared();

	m_pColliderComponent = m_pEntity->CreateComponent<CColliderComponent>();
	m_pData->InitCollider(m_pColliderComponent, rotation);

	if(!m_pColliderComponent->Collide().empty()) {
		return false;
	}

	m_pSpriteComponent = m_pEntity->CreateComponent<CSpriteComponent>();
	m_pSpriteComponent->SetInsBuffersAfterTransform(Frame::Matrix33::CreateTranslation(m_pEntity->GetPosition()) * Frame::Matrix33::CreateRotationZ(m_pEntity->GetRotation()));
	m_pData->InitSprite(m_pSpriteComponent, m_colorUpdatesInSpriteLayers);

	for(auto & def : m_pData->GetConfig().interfaceDefs) {
		SInterface interface;
		interface.ID = def.ID;
		interface.from = this;
		interface.direction = def.direction - m_pEntity->GetRotation();
		interface.pos = m_pEntity->GetPosition()
			+ def.offset.GetRotated(m_pEntity->GetRotation())
			+ Frame::Vec2 { CONNECTOR_LENGTH, 0.f }.GetRotated(-interface.direction)
			;
		m_interfaces.push_back(interface);
	}

	return true;
}

void CEditorDeviceComponent::OnShutDown() {
	for(auto & interface : m_interfaces) {
		DisconnectInterfaces(& interface);
	}
}

void CEditorDeviceComponent::GetConnectorsRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const {
	
	const Frame::SSpriteImage * img = Assets::GetStaticSprite(Assets::EDeviceStaticSprite::connector)->GetImage();
	const auto & buf = Assets::GetImageInstanceBuffer(img);
	const Frame::ColorRGB & color = GetColorSet().connector;

	const Frame::Matrix33 baseTrans = Frame::Matrix33::CreateTranslation(-img->GetOrigin()) * buf.transform;

	for(const auto & interface : m_interfaces) {
		if(!interface.to) {
			continue;
		}
		if(const int normdir = static_cast<int>(Frame::RadToDeg(interface.direction)) % 360; normdir < 5 || normdir >= 185) {
			continue;
		}
		const Frame::Vec2 pos = interface.pos + Frame::Vec2 { -CONNECTOR_HALF_LENGTH, 0.f }.GetRotated(-interface.direction);
		Frame::Matrix33 trans =
			Frame::Matrix33::CreateTranslation(pos)
			* Frame::Matrix33::CreateRotationZ(-interface.direction)
			* baseTrans;
		buffersToPushBack.push_back({ trans, { ONERGB(color), Frame::Min(m_alpha, interface.to->from->GetAlpha()) }, buf.uvMulti, buf.uvAdd });	
	}

}
