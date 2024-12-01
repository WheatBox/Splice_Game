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

	if(!working) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
	{
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
			}
		}
	}
	break;
	case Frame::EntityEvent::EFlag::Render:
	{
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

void CSpriteComponent::GetRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const {
	for(auto & layer : layers) {
		auto it = Assets::gSpriteImageInstanceBufferMap.find(layer.GetCurrentImage());
		if(it == Assets::gSpriteImageInstanceBufferMap.end()) {
			continue;
		}

		// TODO - 应用上该组件的变换等
		buffersToPushBack.push_back(it->second);
	}
}
