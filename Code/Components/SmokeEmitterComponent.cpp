#include "SmokeEmitterComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Depths.h"

REGISTER_ENTITY_COMPONENT(CSmokeEmitterComponent);

CSmokeEmitterComponent * CSmokeEmitterComponent::s_pSmokeEmitterComponent = nullptr;
std::list<CSmokeEmitterComponent::SSmokeParticle> CSmokeEmitterComponent::s_smokePraticles {};

void CSmokeEmitterComponent::Initialize() {
	s_pSmokeEmitterComponent = this;
	m_pEntity->SetZDepth(Depths::SmokeEmitter);
}

Frame::EntityEvent::Flags CSmokeEmitterComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update
		| Frame::EntityEvent::EFlag::Render;
}

void CSmokeEmitterComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
		m_frametime = event.params[0].f;
		break;
	case Frame::EntityEvent::EFlag::Render:
	{
		for(auto it = s_smokePraticles.begin(); it != s_smokePraticles.end();) {
			const Frame::SSpriteImage * pImage = Assets::GetStaticSprite(it->smokeSpr)->GetImage();
			Frame::gRenderer->DrawSpriteBlended(pImage, it->pos, it->color, it->alpha, it->scale, it->rotation);

			it->alpha += it->alphaAdd * m_frametime;
			it->rotation += it->rotationAdd * m_frametime;
			it->pos += it->posAdd * m_frametime;
			it->scale += it->scaleAdd * m_frametime;

			it->pos += it->impulseDir * it->impulsePower * m_frametime;
			it->impulsePower = Lerp(it->impulsePower, 0.f, Frame::Clamp(m_frametime * 10.f, 0.f, 1.f));

			if(it->alpha <= 0.f) {
				it = s_smokePraticles.erase(it);
			} else {
				it++;
			}
		}
	}
	break;
	}
}
