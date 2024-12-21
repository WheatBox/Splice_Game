#include "SpriteComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"
#include "../Assets.h"

REGISTER_ENTITY_COMPONENT(CSpriteComponent);

Frame::EntityEvent::Flags CSpriteComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update | Frame::EntityEvent::EFlag::Render;
}

void CSpriteComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
	{
		if(!bUpdating) {
			break;
		}

		frameTime = event.params[0].f;
		for(auto & layer : layers) {
			layer.Animate(frameTime);
		}

		CheckOrUpdateInsBuffers();
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
	{
		if(!bRendering) {
			break;
		}

		const float entityRot = m_pEntity->GetRotation();
		for(const auto & layer : layers) {
			const Frame::SSpriteImage * pImage = layer.GetCurrentImage();
			if(!pImage) {
				continue;
			}

			const Frame::Vec2 pos = m_pEntity->GetPosition() - layer.GetOffset().GetRotatedDegree(entityRot);
			DrawSpriteBlendedPro(pImage, pos, layer.GetColor(), layer.GetAlpha(), layer.GetRotation(), layer.GetScale(), entityRot);

			if(auto & extraFunc = layer.GetExtraFunction()) {
				extraFunc(frameTime);
			}
		}
	}
	break;
	}
}

void CSpriteComponent::CheckOrUpdateInsBuffers() {
	for(SLayer & layer : layers) {
		if(!layer.__bChanged) {
			continue;
		}
		layer.__bChanged = false;

		const Frame::SSpriteImage * img = layer.GetCurrentImage();
		const auto & buf = Assets::GetImageInstanceBuffer(img);

		const float entRot = m_pEntity->GetRotation();
		const Frame::Vec2 entPos = m_pEntity->GetPosition();

		const Frame::Matrix33 trans =
			Frame::Matrix33::CreateTranslation(entPos - layer.GetOffset().GetRotated(entRot))
			* Frame::Matrix33::CreateRotationZ(entRot)
			* Frame::Matrix33::CreateScale(layer.GetScale())
			* Frame::Matrix33::CreateRotationZ(layer.GetRotation())
			* Frame::Matrix33::CreateTranslation(-img->GetOffset())
			* buf.transform;
		const Frame::ColorRGB col = layer.GetColor();

		m_insBufferGroups[layer.GetInsBufferGroup()][layer.__indexInGroup] = { trans, { ONERGB(col), layer.GetAlpha() }, buf.uvMulti, buf.uvAdd };
	}
}
