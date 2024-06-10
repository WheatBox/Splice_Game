#include "SpriteComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"

REGISTER_ENTITY_COMPONENT(, CSpriteComponent);

Frame::EntityEvent::Flags CSpriteComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update | Frame::EntityEvent::EFlag::Render;
}

void CSpriteComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {

	if(!working) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
	{
		frameTime = event.params[0].f;
		for(auto & layer : layers) {
			if(layer.m_bStatic) {
				break;
			}
			layer.m_frameIntervalCounting += frameTime;
			if(layer.m_frameIntervalCounting >= layer.m_frameInterval) {
				layer.m_frameIntervalCounting -= layer.m_frameInterval;

				layer.m_currentFrame++;
				if(layer.m_currentFrame >= layer.m_frameCount) {
					layer.m_currentFrame -= layer.m_frameCount;
				}
			}
		}
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
	{
		const float entityRot = m_pEntity->GetRotation();
		for(const auto & layer : layers) {
			const Frame::SSpriteImage * pImage = nullptr;
			if(layer.m_bStatic) {
				if(layer.m_pStaticSprite) pImage = layer.m_pStaticSprite->GetImage();
				else break;
			} else {
				if(layer.m_pAnimatedSprite) pImage = layer.m_pAnimatedSprite->GetFrame(layer.m_currentFrame);
				else break;
			}
			const Frame::Vec2 pos = m_pEntity->GetPosition() - layer.m_offset.RotateDegree(entityRot);
			DrawSpriteBlendedPro(pImage, pos, layer.m_color, layer.m_alpha, layer.m_rotation, layer.m_scale, entityRot);

			if(layer.m_extraFunc) {
				layer.m_extraFunc(frameTime);
			}
		}
	}
	break;
	}
}