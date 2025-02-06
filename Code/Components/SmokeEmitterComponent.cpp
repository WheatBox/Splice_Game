#include "SmokeEmitterComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Depths.h"
#include "../Utility.h"

#include <algorithm>

REGISTER_ENTITY_COMPONENT(CSmokeEmitterComponent);

CSmokeEmitterComponent * CSmokeEmitterComponent::s_pSmokeEmitterComponent = nullptr;
CSmokeEmitterComponent::SSmokeParticlesBuffer CSmokeEmitterComponent::s_smokeParticlesBuffer {};
std::vector<CSmokeEmitterComponent::SSmokeParticle> CSmokeEmitterComponent::s_smokeParticles;

const Frame::SSpriteImage * CSmokeEmitterComponent::s_images[5] {};
Frame::Vec2 CSmokeEmitterComponent::s_uvAdds[5] {};
Frame::STextureVertexBuffer CSmokeEmitterComponent::s_defaultTexVertBuf {};

void CSmokeEmitterComponent::Initialize() {
	s_pSmokeEmitterComponent = this;
	m_pEntity->SetZDepth(Depths::SmokeEmitter);

	for(int i = 0; i < 5; i++) {
		if(s_images[i]) {
			continue;
		}
		s_images[i] = Assets::GetStaticSprite(SSmokeParticle::sprites[i])->GetImage();
		s_uvAdds[i] = Assets::GetImageInstanceBuffer(s_images[i]).uvAdd;
	}
	const Frame::SSpriteImage * img = CSmokeEmitterComponent::s_images[0];
	s_defaultTexVertBuf.SetPositions(img->GetTopLeftOffset(), img->GetBottomRightOffset());
	s_defaultTexVertBuf.SetBlends(0xFFFFFF, 1.f);
	s_defaultTexVertBuf.SetTexCoord(0.f, img->GetUVRightBottom() - img->GetUVLeftTop());
}

Frame::EntityEvent::Flags CSmokeEmitterComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::Render;
}

std::vector<float> times;
float sec = 0.f;
float fpssec = 0.f;

void CSmokeEmitterComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
		m_frametime = event.params[0].f;
		break;
	case Frame::EntityEvent::EFlag::Render:
	{
		std::vector<Frame::CRenderer::SInstanceBuffer> instances;

		auto & smokeParticles = s_smokeParticlesBuffer.buffers[s_smokeParticlesBuffer.usingBufferId];
		//s_smokeParticlesBuffer.usingBufferId = !s_smokeParticlesBuffer.usingBufferId; // 这么写 VS 会给我弹提示，不好看
		s_smokeParticlesBuffer.usingBufferId = s_smokeParticlesBuffer.usingBufferId == 0 ? 1 : 0;

		s_smokeParticles.insert(s_smokeParticles.end(), smokeParticles.begin(), smokeParticles.end());
		smokeParticles.clear();

		auto itToErase = std::remove_if(s_smokeParticles.begin(), s_smokeParticles.end(), [this, & instances](SSmokeParticle & part) {
			part.alpha += part.alphaAdd * m_frametime;
			part.rotation += part.rotationAdd * m_frametime;
			part.pos += part.posAdd * m_frametime;
			part.scale += part.scaleAdd * m_frametime;

			part.pos += part.impulseDir * part.impulsePower * m_frametime;
			part.impulsePower = Lerp(part.impulsePower, 0.f, Frame::Clamp(m_frametime * 10.f, 0.f, 1.f));

			if(part.alpha > 0.f) {
				instances.push_back({
					Frame::Matrix33::CreateTranslation(part.pos) * Frame::Matrix33::CreateRotationZ(part.rotation) * Frame::Matrix33::CreateScale(part.scale)
					, { ONERGB(part.color), part.alpha }
					, 1.f, CSmokeEmitterComponent::s_uvAdds[part.smokeSprIndex]
					});
				return false;
			}
			return true;
			});
		s_smokeParticles.erase(itToErase, s_smokeParticles.end());

		Frame::gRenderer->DrawTexturesInstanced(CSmokeEmitterComponent::s_images[0]->GetTextureId(), CSmokeEmitterComponent::s_defaultTexVertBuf, instances);

		sec += m_frametime;
		times.push_back(m_frametime);
		if(sec >= 1.f) {
			sec = 0.f;
			fpssec = 0.f;
			for(auto & time : times) {
				fpssec += 1.f / time;
			}
			fpssec /= (float)times.size();
			times.clear();
		}
		Frame::gRenderer->pTextRenderer->DrawText(std::string { "fps: " } + std::to_string((int)std::round(fpssec)), Frame::gCamera->WindowToScene(0.f));
	}
	break;
	}
}
