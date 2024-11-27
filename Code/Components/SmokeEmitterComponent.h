#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>
#include <FrameMath/Matrix33.h>

#include "../Assets.h"

#include <cstdlib>
#include <list>

class CSmokeEmitterComponent : public Frame::IEntityComponent {
public:

	struct SSmokeParticle {
		SSmokeParticle(const Frame::Vec2 & _pos, float _scale)
			: SSmokeParticle(_pos, _scale, 0x444444)
		{}
		SSmokeParticle(const Frame::Vec2 & _pos, float _scale, Frame::ColorRGB _color)
			: SSmokeParticle(_pos, _scale, _color, 0.f)
		{}
		SSmokeParticle(const Frame::Vec2 & _pos, float _scale, Frame::ColorRGB _color, const Frame::Vec2 & _impulse)
			: pos { _pos }
			, scale { _scale }
			, color { _color }
		{
			smokeSprIndex = rand() % spritesCount;

			if(_impulse.x != 0 || _impulse.y != 0) {
				impulseDir = _impulse.GetNormalized();
				impulsePower = _impulse.Length();
			}

			/*scale *= static_cast<float>(rand() % 8) * .1f + .6f;
			rotation = static_cast<float>(rand() % 360);
			alpha = static_cast<float>(rand() % 5) * .1f;

			posAdd = Frame::Vec2 { static_cast<float>(rand() % 20 + 30), 0.f }.RotateDegree(static_cast<float>(rand() % 360)) * (static_cast<float>(rand() % 6) * .1f + .4f);
			scaleAdd = static_cast<float>(rand() % 5) * .1f + .5f;
			rotationAdd = static_cast<float>(rand() % 75 * (rand() % 2 ? -1 : 1));
			alphaAdd = -static_cast<float>(rand() % 2) * .1f - .2f;*/

			scale *= static_cast<float>(rand() % 8 + 6) * .1f;
			rotation = Frame::DegToRad(static_cast<float>(rand() % 360));
			alpha = static_cast<float>(rand() % 3 + 1) * .1f;

			posAdd = Frame::Vec2 { static_cast<float>(rand() % 40 + 30), 0.f }.GetRotatedDegree(static_cast<float>(rand() % 360)) * (static_cast<float>(rand() % 6 + 4) * .1f);
			scaleAdd = static_cast<float>(rand() % 8 + 10) * .1f;
			rotationAdd = Frame::DegToRad(static_cast<float>(rand() % 75 * (rand() % 2 ? -1 : 1)));
			alphaAdd = -static_cast<float>(rand() % 2 + 2) * .1f;
		}
		Frame::Vec2 pos;
		float scale;
		Frame::ColorRGB color;

		Frame::Vec2 impulseDir {};
		float impulsePower = 0.f;

		int smokeSprIndex;

		float rotation;
		float alpha;

		Frame::Vec2 posAdd;
		float scaleAdd;
		float rotationAdd;
		float alphaAdd;

		static constexpr Assets::EOtherStaticSprite sprites[] = {
			Assets::EOtherStaticSprite::smoke1,
			Assets::EOtherStaticSprite::smoke2,
			Assets::EOtherStaticSprite::smoke3,
			Assets::EOtherStaticSprite::smoke4,
			Assets::EOtherStaticSprite::smoke5
		};
		static constexpr int spritesCount = 5;

		static Frame::Matrix33 spritesTexCoordTrans[spritesCount];
	};

	static void SummonSmokeParticle(const SSmokeParticle & particle) {
		s_smokeParticlesBuffer.buffers[s_smokeParticlesBuffer.usingBufferId].push_back(particle);
	}

	struct SSmokeParticlesBuffer {
		std::vector<SSmokeParticle> buffers[2] {};
		int usingBufferId = 0;
	};
	static SSmokeParticlesBuffer s_smokeParticlesBuffer; // 两个缓存交替给 s_smokeParticles 传递新烟雾粒子的数据，用以解决多线程方面的冲突问题
	static std::vector<SSmokeParticle> s_smokeParticles;

public:
	virtual void Initialize() override;
	virtual void OnShutDown() override {
		if(s_pSmokeEmitterComponent == this) {
			s_pSmokeEmitterComponent = nullptr;
		}
	}
	static CSmokeEmitterComponent * s_pSmokeEmitterComponent;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CSmokeEmitterComponent> type) {
		type.SetGUID("{A52B0B3F-CD66-4349-A565-749971515235}");
	}

private:
	float m_frametime = 0.f;
};
