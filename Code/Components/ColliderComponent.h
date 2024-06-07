#pragma once

#include <FrameEntity/IEntityComponent.h>

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include <unordered_set>
#include <list>

class CColliderComponent : public Frame::IEntityComponent {
public:

	/*
	enum class EShape {
		Rectangle,
		Circle
	};
	*/
	
	static std::unordered_set<CColliderComponent *> s_colliders;

	virtual void Initialize() override {
		s_colliders.insert(this);
	}
	virtual void OnShutDown() override {
		if(auto it = s_colliders.find(this); it != s_colliders.end()) {
			s_colliders.erase(it);
		}
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override {
		return Frame::EntityEvent::EFlag::Nothing;
	}
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent &) override {}

	static void Register(Frame::SComponentType<CColliderComponent> type) {
		type.SetGUID("{58E783BF-B81B-4D2D-AAFA-208077AD8523}");
	}

	struct SCollider {
		SCollider() = default;
		SCollider(const Frame::Vec2 & _siz, const Frame::Vec2 & _off)
			: size { _siz }
			, offset { _off }
		{}

		Frame::Vec2 size { 1.f };
		Frame::Vec2 offset { 0.f };
	};

	std::list<CColliderComponent *> Collide() const;

	void Render(const Frame::ColorRGB & color, float alpha) const;

	std::vector<SCollider> m_colliders;

	void AddCollider(const SCollider & collider) {
		m_colliders.push_back(collider);
	}

};