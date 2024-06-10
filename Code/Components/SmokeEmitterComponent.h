#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameMath/Vector2.h>

#include <cstdlib>
#include <list>

class CSmokeEmitterComponent : public Frame::IEntityComponent {
public:

	struct SSmokeParticle {
		SSmokeParticle(const Frame::Vec2 & _pos, float _scale)
			: pos { _pos }
			, scale { _scale }
		{
			smokeSprIndex = rand() % 5;

			scale *= static_cast<float>(rand() % 8) * .1f + .6f;
			rotation = static_cast<float>(rand() % 360);
			alpha = static_cast<float>(rand() % 5) * .1f;

			posAdd = Frame::Vec2 { static_cast<float>(rand() % 20 + 30), 0.f }.RotateDegree(static_cast<float>(rand() % 360)) * (static_cast<float>(rand() % 6) * .1f + .4f);
			scaleAdd = static_cast<float>(rand() % 5) * .1f + .5f;
			rotationAdd = static_cast<float>(rand() % 75 * (rand() % 2 ? -1 : 1));
			alphaAdd = -static_cast<float>(rand() % 2) * .1f - .2f;
		}
		Frame::Vec2 pos;
		float scale;

		int smokeSprIndex = 0;

		float rotation;
		float alpha;

		Frame::Vec2 posAdd;
		float scaleAdd;
		float rotationAdd;
		float alphaAdd;
	};

	static void SummonSmokeParticle(const SSmokeParticle & particle) {
		s_smokePraticles.push_front(particle); // 后创建的粒子先绘制
	}
	static CSmokeEmitterComponent * s_pSmokeEmitterComponent;
	static std::list<SSmokeParticle> s_smokePraticles;

public:
	virtual void Initialize() override;
	virtual void OnShutDown() override {
		if(s_pSmokeEmitterComponent == this) {
			s_pSmokeEmitterComponent = nullptr;
		}
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CSmokeEmitterComponent> type) {
		type.SetGUID("{A52B0B3F-CD66-4349-A565-749971515235}");
	}

private:
	float m_frametime = 0.f;
};