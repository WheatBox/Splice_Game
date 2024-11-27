#include "SmokeEmitterComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Depths.h"

REGISTER_ENTITY_COMPONENT(CSmokeEmitterComponent);

Frame::Matrix33 CSmokeEmitterComponent::SSmokeParticle::spritesTexCoordTrans[spritesCount] {};

CSmokeEmitterComponent * CSmokeEmitterComponent::s_pSmokeEmitterComponent = nullptr;
std::vector<CSmokeEmitterComponent::SSmokeParticle> CSmokeEmitterComponent::s_smokePraticles {};

void CSmokeEmitterComponent::Initialize() {
	s_pSmokeEmitterComponent = this;
	m_pEntity->SetZDepth(Depths::SmokeEmitter);

	const Frame::Vec2 texCoordBase = Assets::GetStaticSprite(SSmokeParticle::sprites[0])->GetImage()->GetUVLeftTop();
	SSmokeParticle::spritesTexCoordTrans[0] = Frame::Matrix33::CreateIdentity();
	for(int i = 1; i < 5; i++) {
		SSmokeParticle::spritesTexCoordTrans[i] = Frame::Matrix33::CreateTranslation(Assets::GetStaticSprite(SSmokeParticle::sprites[i])->GetImage()->GetUVLeftTop() - texCoordBase);
	}
}

Frame::EntityEvent::Flags CSmokeEmitterComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::Render;
}

std::vector<float> times;
float sec = 0.f;
float fpssec = 0.f;

#include <chrono>
#include <iostream>

void CSmokeEmitterComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
		m_frametime = event.params[0].f;
		break;
	case Frame::EntityEvent::EFlag::Render:
	{
		std::vector<Frame::CRenderer::SInstanceBuffer> instances;

		auto itToErase = std::remove_if(s_smokePraticles.begin(), s_smokePraticles.end(), [this, & instances](SSmokeParticle & part) {
			part.alpha += part.alphaAdd * m_frametime;
			part.rotation += part.rotationAdd * m_frametime;
			part.pos += part.posAdd * m_frametime;
			part.scale += part.scaleAdd * m_frametime;

			part.pos += part.impulseDir * part.impulsePower * m_frametime;
			part.impulsePower = Lerp(part.impulsePower, 0.f, Frame::Clamp(m_frametime * 10.f, 0.f, 1.f));

			if(part.alpha > 0.f) {
				instances.push_back({
					Frame::Matrix33::CreateTranslation(part.pos) * Frame::Matrix33::CreateRotationZ(part.rotation) * Frame::Matrix33::CreateScale(part.scale)
					, SSmokeParticle::spritesTexCoordTrans[part.smokeSprIndex]
					, { ONERGB(part.color), part.alpha }
					});
				return false;
			}
			return true;
			});
		s_smokePraticles.erase(itToErase, s_smokePraticles.end());

		Frame::gRenderer->DrawSpritesInstanced(Assets::GetStaticSprite(SSmokeParticle::sprites[0])->GetImage(), instances);

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
