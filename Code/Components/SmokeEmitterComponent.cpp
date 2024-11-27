#include "SmokeEmitterComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Depths.h"

REGISTER_ENTITY_COMPONENT(CSmokeEmitterComponent);

Frame::Matrix33 CSmokeEmitterComponent::SSmokeParticle::spritesTexCoordTrans[spritesCount] {};

CSmokeEmitterComponent * CSmokeEmitterComponent::s_pSmokeEmitterComponent = nullptr;
std::list<CSmokeEmitterComponent::SSmokeParticle> CSmokeEmitterComponent::s_smokePraticles {};

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

// 开始计时
auto start = std::chrono::high_resolution_clock::now();

		std::vector<Frame::CRenderer::SInstanceBuffer> instances;

		for(auto it = s_smokePraticles.begin(); it != s_smokePraticles.end();) {
			it->alpha += it->alphaAdd * m_frametime;
			it->rotation += it->rotationAdd * m_frametime;
			it->pos += it->posAdd * m_frametime;
			it->scale += it->scaleAdd * m_frametime;

			it->pos += it->impulseDir * it->impulsePower * m_frametime;
			it->impulsePower = Lerp(it->impulsePower, 0.f, Frame::Clamp(m_frametime * 10.f, 0.f, 1.f));

			if(it->alpha <= 0.f) {
				it = s_smokePraticles.erase(it);
			} else {

				instances.push_back({
					Frame::Matrix33::CreateTranslation(it->pos) * Frame::Matrix33::CreateRotationZ(it->rotation) * Frame::Matrix33::CreateScale(it->scale)
					, SSmokeParticle::spritesTexCoordTrans[it->smokeSprIndex]
					, { ONERGB(it->color), it->alpha }
					});

				instances.push_back({});
				it++;
			}
		}

// 获取结束时间点
auto end = std::chrono::high_resolution_clock::now();
// 计算运行时间，以毫秒为单位
std::chrono::duration<double, std::milli> elapsed = end - start;
// 输出运行时间
std::cout << "运行时间: " << elapsed.count() << " 毫秒" << std::endl;

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
