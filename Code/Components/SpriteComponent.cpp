﻿#include "SpriteComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"
#include "../Assets.h"

REGISTER_ENTITY_COMPONENT(CSpriteComponent);

void CSprite::DrawAllLayers(const Frame::Vec2 & pos, const float rot, const float frameTime) const {
	for(const auto & layer : layers) {
		const Frame::SSpriteImage * pImage = layer.GetCurrentImage();
		if(!pImage) {
			continue;
		}

		DrawSpriteBlendedPro(pImage, pos + layer.GetOffset().GetRotated(rot), layer.GetColor(), layer.GetAlpha(), layer.GetRotation(), layer.GetScale(), rot);

		if(auto & extraFunc = layer.GetExtraFunction()) {
			extraFunc(frameTime);
		}
	}
}

void CSprite::CheckOrUpdateInsBuffers() {
	for(SLayer & layer : layers) {
		if(!layer.__bChanged) {
			continue;
		}
		layer.__bChanged = false;

		const Frame::SSpriteImage * img = layer.GetCurrentImage();
		const auto & buf = Assets::GetImageInstanceBuffer(img);

		const Frame::Matrix33 trans =
			m_insBuffersAfterTransform *
			Frame::Matrix33::CreateTranslation(layer.GetOffset()) *
			Frame::Matrix33::CreateScale(layer.GetScale()) *
			Frame::Matrix33::CreateRotationZ(layer.GetRotation()) *
			Frame::Matrix33::CreateTranslation(-img->GetOrigin()) *
			buf.transform;
		const Frame::ColorRGB col = layer.GetColor();

		m_insBufferGroups[layer.GetInsBufferGroup()][layer.__indexInGroup] = { trans, { ONERGB(col), layer.GetAlpha() }, buf.uvMulti, buf.uvAdd };
	}
}

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
		sprite.AnimateAllLayers(frameTime);

		CheckOrUpdateInsBuffers();
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
	{
		if(!bRendering) {
			break;
		}

		sprite.DrawAllLayers(m_pEntity->GetPosition(), m_pEntity->GetRotation(), frameTime);
	}
	break;
	}
}
