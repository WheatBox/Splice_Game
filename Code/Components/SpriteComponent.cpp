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
			if(!layer.m_bAnimated) {
				break;
			}
			layer.m_frameIntervalCounting += frameTime;
			if(layer.m_frameIntervalCounting >= layer.m_frameInterval) {
				layer.m_frameIntervalCounting -= layer.m_frameInterval;

				layer.m_currentFrame++;
				if(layer.m_currentFrame >= layer.m_frameCount) {
					layer.m_currentFrame -= layer.m_frameCount;
				}

				layer.__Changed();
			}
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

			const Frame::Vec2 pos = m_pEntity->GetPosition() - layer.m_offset.GetRotatedDegree(entityRot);
			DrawSpriteBlendedPro(pImage, pos, layer.m_color, layer.m_alpha, layer.m_rotation, layer.m_scale, entityRot);

			if(layer.m_extraFunc) {
				layer.m_extraFunc(frameTime);
			}
		}
	}
	break;
	}
}

void CSpriteComponent::CheckOrUpdateInsBuffers() {
	for(int i = 0, siz = static_cast<int>(layers.size()); i < siz; i++) {
		SLayer & layer = layers[i];

		if(!layer.__bChanged) {
			continue;
		}
		layer.__bChanged = false;

		const Frame::SSpriteImage * img = layer.GetCurrentImage();
		const auto & buf = Assets::GetImageInstanceBuffer(img);

		const float entRot = m_pEntity->GetRotation();
		const Frame::Vec2 entPos = m_pEntity->GetPosition();

		const Frame::Matrix33 trans =
			Frame::Matrix33::CreateTranslation(entPos - layer.GetOffset().GetRotatedDegree(entRot))
			* Frame::Matrix33::CreateRotationZ(Frame::DegToRad(entRot))
			* Frame::Matrix33::CreateScale(layer.GetScale())
			* Frame::Matrix33::CreateRotationZ(Frame::DegToRad(layer.GetRotation()))
			* Frame::Matrix33::CreateTranslation(-img->GetOffset())
			* buf.transform;
		const Frame::ColorRGB col = layer.GetColor();

		if(static_cast<int>(m_insBuffers.size()) <= i) {
			m_insBuffers.push_back({ trans, { ONERGB(col), layer.GetAlpha() }, buf.uvMulti, buf.uvAdd });
		} else {
			m_insBuffers[i] = { trans, { ONERGB(col), layer.GetAlpha() }, buf.uvMulti, buf.uvAdd };
		}
	}
}
