#pragma once

#include <FrameEntity/IEntityComponent.h>

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include "../box2dIncluded.h"
#include "../Application.h"

class CDeviceComponent;

class CRigidbodyComponent : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override {
		return Frame::EntityEvent::EFlag::Render;
	}
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent &) override;

	static void Register(Frame::SComponentType<CRigidbodyComponent> type) {
		type.SetGUID("{CDA353C9-A3BE-456F-BF3F-71F1CC2D4BF2}");
	}

	void Physicalize(const b2BodyDef & bodyDef, const b2FixtureDef & fixtureDef);
	void Physicalize(const b2BodyDef & bodyDef, const std::vector<b2FixtureDef *> & fixtureDefs);
	void Physicalize(const b2BodyDef & bodyDef, const std::vector<b2FixtureDef *> & fixtureDefs, const std::unordered_map<b2FixtureDef *, CDeviceComponent *> & map_fixtureDefDeviceComp);

	void SetEnableRendering(bool enable) {
		bRender = enable;
	}

	// TODO
	void WeldWith(const CRigidbodyComponent * pRigidbodyComponent);

	b2Body * GetBody() const {
		return m_pBody;
	}

	Frame::Vec2 GetPosition() const {
		const b2Vec2 pos = m_pBody->GetPosition();
		return MeterToPixelVec2({ pos.x, pos.y });
	}

	float GetRotation() const {
		return Frame::RadToDeg(m_pBody->GetAngle());
	}

	void ApplyForce(const Frame::Vec2 & force) {
		m_pBody->ApplyForceToCenter({ force.x, force.y }, true);
	}
	void ApplyForce(const Frame::Vec2 & force, const Frame::Vec2 & pointInWorld_pixel) {
		m_pBody->ApplyForce({ force.x, force.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyLinearImpulseToCenter(const Frame::Vec2 & impulse) {
		m_pBody->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
	}
	void ApplyLinearImpulse(const Frame::Vec2 & impulse, const Frame::Vec2 & pointInWorld_pixel) {
		m_pBody->ApplyLinearImpulse({ impulse.x, impulse.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyAngularImpulse(float impulse) {
		m_pBody->ApplyAngularImpulse(impulse, true);
	}

private:

	b2Body * m_pBody = nullptr;
	std::vector<b2Fixture *> m_fixtures;
	bool bRender = false;

};