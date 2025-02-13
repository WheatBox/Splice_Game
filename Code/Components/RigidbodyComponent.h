#pragma once

#include <FrameEntity/IEntityComponent.h>

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include <box2d/box2d.h>

#include "../Application.h"

#include <functional>
#include <variant>
#include <memory>
#include <unordered_set>

struct IDeviceData;

class CRigidbodyComponent : public Frame::IEntityComponent {
public:

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{CDA353C9-A3BE-456F-BF3F-71F1CC2D4BF2}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override {
		return Frame::EntityEvent::EFlag::Render;
	}
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent &) override;

	virtual void OnShutDown() override;

	struct SBox2dShape {
		std::variant<b2Polygon, b2Circle> shape;
		b2ShapeType type;
	};

	void Physicalize(b2BodyDef bodyDef, b2ShapeDef shapeDef, SBox2dShape shape);
	void Physicalize(b2BodyDef bodyDef, std::vector<std::pair<b2ShapeDef, SBox2dShape>> shapeDefs);
	void Physicalize(b2BodyDef bodyDef, std::unordered_set<std::shared_ptr<IDeviceData>> devices);

	void SetEnableRendering(bool enable) {
		bRender = enable;
	}
	void SetRenderingColor(int rgb) {
		m_renderColor = rgb;
	}
	void SetRenderingColorAlwaysLight(bool enable) {
		m_renderColorAlwaysLight = enable;
	}

	// 如果要连接的实体与该组件所在实体为同一实体，返回 false，否则返回 true
	bool CreateJointWith(Frame::EntityId entityId, std::function<void (b2BodyId, b2BodyId)> funcCreateJoint);

	const b2BodyId & GetBody() const {
		return m_bodyId;
	}
	void SetBody(const b2BodyId & bodyId) {
		m_bodyId = bodyId;
	}

	bool IsBodyValid() const {
		return b2Body_IsValid(m_bodyId);
	}

	std::vector<b2ShapeId> & GetShapes() {
		return m_shapes;
	}

	Frame::Vec2 GetPosition() const {
		if(!IsBodyValid()) {
			return { 0.f, 0.f };
		}
		const b2Vec2 pos = b2Body_GetPosition(m_bodyId);
		return MeterToPixelVec2({ pos.x, pos.y });
	}

	float GetRotation() const {
		if(!IsBodyValid()) {
			return 0.f;
		}
		return b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
	}

	void ApplyForce(const Frame::Vec2 & force) {
		if(!IsBodyValid()) {
			return;
		}
		b2Body_ApplyForceToCenter(m_bodyId, { force.x, force.y }, true);
	}
	void ApplyForce(const Frame::Vec2 & force, const Frame::Vec2 & pointInWorld_pixel) {
		if(!IsBodyValid()) {
			return;
		}
		b2Body_ApplyForce(m_bodyId, { force.x, force.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyLinearImpulseToCenter(const Frame::Vec2 & impulse) {
		if(!IsBodyValid()) {
			return;
		}
		b2Body_ApplyLinearImpulseToCenter(m_bodyId, { impulse.x, impulse.y }, true);
	}
	void ApplyLinearImpulse(const Frame::Vec2 & impulse, const Frame::Vec2 & pointInWorld_pixel) {
		if(!IsBodyValid()) {
			return;
		}
		b2Body_ApplyLinearImpulse(m_bodyId, { impulse.x, impulse.y }, { PixelToMeter(pointInWorld_pixel.x), PixelToMeter(pointInWorld_pixel.y) }, true);
	}

	void ApplyAngularImpulse(float impulse) {
		if(!IsBodyValid()) {
			return;
		}
		b2Body_ApplyAngularImpulse(m_bodyId, impulse, true);
	}

private:

	b2BodyId m_bodyId;
	std::vector<b2ShapeId> m_shapes;
	bool bRender = false;
	int m_renderColor = 0xFF7E00;
	bool m_renderColorAlwaysLight = false;

};

using SBox2dShape = CRigidbodyComponent::SBox2dShape;