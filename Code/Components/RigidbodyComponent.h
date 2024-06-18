#pragma once

#include <FrameEntity/IEntityComponent.h>

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include "../box2dIncluded.h"
#include "../Application.h"

#include <functional>

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

	virtual void OnShutDown() override;

	void Physicalize(b2BodyDef bodyDef, b2FixtureDef fixtureDef);
	// 运行结束后，会自动释放 fixtureDefs 中的 b2FixtureDef
	void Physicalize(b2BodyDef bodyDef, std::vector<b2FixtureDef *> fixtureDefs);
	// 运行结束后，会自动释放 fixtureDefs 中的 b2FixtureDef
	void Physicalize(b2BodyDef bodyDef, std::vector<b2FixtureDef *> fixtureDefs, std::unordered_map<b2FixtureDef *, CDeviceComponent *> map_fixtureDefDeviceComp);

	void SetEnableRendering(bool enable) {
		bRender = enable;
	}

	// 如果要连接的实体与该组件所在实体为同一实体，返回 false，否则返回 true
	bool CreateJointWith(Frame::EntityId entityId, std::function<b2JointDef * (b2Body *, b2Body *)> funcCreateJointDef, std::function<void (b2JointDef *)> funcDestroyJointDef);

	b2Body * GetBody() const {
		return m_pBody;
	}
	void SetBody(b2Body * pBody) {
		m_pBody = pBody;
	}

	std::vector<b2Fixture *> & GetFixtures() {
		return m_fixtures;
	}

	Frame::Vec2 GetPosition() const {
		if(!m_pBody) {
			return { 0.f, 0.f };
		}
		const b2Vec2 pos = m_pBody->GetPosition();
		return MeterToPixelVec2({ pos.x, pos.y });
	}

	float GetRotation() const {
		if(!m_pBody) {
			return 0.f;
		}
		return Frame::RadToDeg(m_pBody->GetAngle());
	}

	void ApplyForce(const Frame::Vec2 & force) {
		if(!m_pBody) {
			return;
		}
		m_pBody->ApplyForceToCenter({ force.x, force.y }, true);
	}
	void ApplyForce(const Frame::Vec2 & force, const Frame::Vec2 & pointInWorld_pixel) {
		if(!m_pBody) {
			return;
		}
		m_pBody->ApplyForce({ force.x, force.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyLinearImpulseToCenter(const Frame::Vec2 & impulse) {
		if(!m_pBody) {
			return;
		}
		m_pBody->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
	}
	void ApplyLinearImpulse(const Frame::Vec2 & impulse, const Frame::Vec2 & pointInWorld_pixel) {
		if(!m_pBody) {
			return;
		}
		m_pBody->ApplyLinearImpulse({ impulse.x, impulse.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyAngularImpulse(float impulse) {
		if(!m_pBody) {
			return;
		}
		m_pBody->ApplyAngularImpulse(impulse, true);
	}

private:

	b2Body * m_pBody = nullptr;
	std::vector<b2Fixture *> m_fixtures;
	bool bRender = false;

};