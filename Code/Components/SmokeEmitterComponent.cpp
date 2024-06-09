#include "SmokeEmitterComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Assets.h"
#include "../Depths.h"

REGISTER_ENTITY_COMPONENT(, CSmokeEmitterComponent);

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
		constexpr Assets::EOtherStaticSprite sprs[] = {
			Assets::EOtherStaticSprite::smoke1,
			Assets::EOtherStaticSprite::smoke2,
			Assets::EOtherStaticSprite::smoke3,
			Assets::EOtherStaticSprite::smoke4,
			Assets::EOtherStaticSprite::smoke5
		};
		for(auto it = s_smokePraticles.begin(); it != s_smokePraticles.end();) {
			const Frame::SSpriteImage * pImage = Assets::GetStaticSprite(sprs[it->smokeSprIndex])->GetImage();
			Frame::gRenderer->DrawSpriteBlended(pImage, it->pos, 0x444444, it->alpha, it->rotation, it->scale);

			it->alpha += it->alphaAdd * m_frametime;
			it->rotation += it->rotationAdd * m_frametime;
			it->pos += it->posAdd * m_frametime;
			it->scale += it->scaleAdd * m_frametime;

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
