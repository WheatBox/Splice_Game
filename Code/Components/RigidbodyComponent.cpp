#include "RigidbodyComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Application.h"
#include "../Devices/IDevicesData.h"

#include "PhysicsWorldComponent.h"

REGISTER_ENTITY_COMPONENT(CRigidbodyComponent);

void CRigidbodyComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Render:
	{
		if(!bRender || !b2Body_IsValid(m_bodyId)) {
			break;
		}

		const int color = (m_renderColorAlwaysLight || b2Body_IsAwake(m_bodyId)) ? m_renderColor : 0xB4B4B4;

		for(const auto & shapeId : m_shapes) {
			if(!b2Shape_IsValid(shapeId)) {
				continue;
			}

			b2ShapeType shapeType = b2Shape_GetType(shapeId);
			switch(shapeType) {
			case b2ShapeType::b2_polygonShape:
			{
				b2Polygon polygon = b2Shape_GetPolygon(shapeId);

				float rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
				b2Vec2 bodyPos = b2Body_GetPosition(m_bodyId);
				Frame::Vec2 posAdd { bodyPos.x, bodyPos.y };

				for(int i = 2; i < polygon.count; i++) {
					Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { polygon.vertices[0].x, polygon.vertices[0].y }.GetRotated(rot)),
						MeterToPixelVec2(posAdd + Frame::Vec2 { polygon.vertices[i].x, polygon.vertices[i].y }.GetRotated(rot)),
						MeterToPixelVec2(posAdd + Frame::Vec2 { polygon.vertices[i - 1].x, polygon.vertices[i - 1].y }.GetRotated(rot)),
						color, .5f
					);
				}
				for(int i = 0; i < polygon.count; i++) {
					b2Vec2 pos = polygon.vertices[i];
					b2Vec2 posPrev = (i == 0 ? polygon.vertices[polygon.count - 1] : polygon.vertices[i - 1]);
					Frame::gRenderer->pShapeRenderer->DrawLineBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { posPrev.x, posPrev.y }.GetRotated(rot)),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pos.x, pos.y }.GetRotated(rot)),
						color, 1.f
					);
				}
			}
			break;
			case b2ShapeType::b2_circleShape:
			{
				b2Circle circle = b2Shape_GetCircle(shapeId);
				
				float rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
				b2Vec2 bodyPos = b2Body_GetPosition(m_bodyId);
				Frame::Vec2 posAdd = Frame::Vec2 { bodyPos.x, bodyPos.y } + Frame::Vec2 { circle.center.x, circle.center.y }.GetRotated(rot);

				constexpr int vertCount = 12;
				constexpr float angleAdd = Frame::DegToRad(360.f / static_cast<float>(vertCount));

				float angle = 2.f * angleAdd;
				b2Vec2 posPrev { std::cos(angleAdd) * circle.radius, std::sin(angleAdd) * circle.radius };
				b2Vec2 posFirst { circle.radius, 0.f };
				for(int i = 2; i < vertCount; i++) {
					b2Vec2 pos { std::cos(angle) * circle.radius, std::sin(angle) * circle.radius };
					Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { posFirst.x, posFirst.y }),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pos.x, pos.y }),
						MeterToPixelVec2(posAdd + Frame::Vec2 { posPrev.x, posPrev.y }),
						color, .5f
					);
					posPrev = pos;
					angle += angleAdd;
				}

				angle = 0.f;
				posPrev = { std::cos(-angleAdd) * circle.radius, std::sin(-angleAdd) * circle.radius };
				for(int i = 0; i < vertCount; i++) {
					b2Vec2 pos { std::cos(angle) * circle.radius, std::sin(angle) * circle.radius };
					Frame::gRenderer->pShapeRenderer->DrawLineBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { posPrev.x, posPrev.y }),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pos.x, pos.y }),
						color, 1.f
					);
					posPrev = pos;
					angle += angleAdd;
				}
			}
			break;
			}
		}
	}
	break;
	}
}

void CRigidbodyComponent::OnShutDown() {
	if(IsBodyValid()) {
		b2BodyId bodyId = m_bodyId;
		CPhysicsWorldComponent::s_physicalizeQueue.push(
			[bodyId]() { b2DestroyBody(bodyId); }
		);
	}
}

static inline b2ShapeId CreateShape(const b2BodyId & bodyId, const b2ShapeDef & shapeDef, const CRigidbodyComponent::SBox2dShape & shape) {
	switch(shape.type) {
	case b2ShapeType::b2_circleShape:
		return b2CreateCircleShape(bodyId, & shapeDef, & std::get<b2Circle>(shape.shape));
	case b2ShapeType::b2_polygonShape:
		return b2CreatePolygonShape(bodyId, & shapeDef, & std::get<b2Polygon>(shape.shape));
	}
	return b2_nullShapeId;
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, b2ShapeDef shapeDef, SBox2dShape shape) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, shapeDef, shape]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2BodyId bodyId = b2CreateBody(gWorldId, & bodyDef); b2Body_IsValid(bodyId)) {
						pComp->SetBody(bodyId);
						pComp->GetShapes().push_back(CreateShape(bodyId, shapeDef, shape));
					}
				}
			}
		}
	);
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, std::vector<std::pair<b2ShapeDef, SBox2dShape>> shapeDefs) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, shapeDefs]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2BodyId bodyId = b2CreateBody(gWorldId, & bodyDef); b2Body_IsValid(bodyId)) {
						pComp->SetBody(bodyId);
						for(const auto & shapeDef : shapeDefs) {
							pComp->GetShapes().push_back(CreateShape(bodyId, shapeDef.first, shapeDef.second));
						}
					}
				}
			}
		}
	);
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, std::unordered_set<std::shared_ptr<IDeviceData>> devices) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, devices]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2BodyId bodyId = b2CreateBody(gWorldId, & bodyDef); b2Body_IsValid(bodyId)) {
						pComp->SetBody(bodyId);
						for(const auto & device : devices) {
							const auto shapeDefs = device->MakeShapeDefs(device->m_positionRelative, device->m_rotationRelative);
							for(const auto & shapeDef : shapeDefs) {
								b2ShapeId shape = CreateShape(bodyId, shapeDef.first, shapeDef.second);
								pComp->GetShapes().push_back(shape);
								device->m_shapes.push_back(shape);
							}
						}
					}
				}
			}
		}
	);
}

bool CRigidbodyComponent::CreateJointWith(Frame::EntityId entityId, std::function<void (b2BodyId, b2BodyId)> funcCreateJoint) {
	Frame::EntityId myEntityId = m_pEntity->GetId();

	if(myEntityId == entityId) {
		return false;
	}
	
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, entityId, funcCreateJoint]() {
			b2BodyId myBody = b2_nullBodyId, anotherBody = b2_nullBodyId;
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					myBody = pComp->GetBody();
				}
			}
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(entityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					anotherBody = pComp->GetBody();
				}
			}

			if(b2Body_IsValid(myBody) && b2Body_IsValid(anotherBody)) {
				funcCreateJoint(myBody, anotherBody);
			}
		}
	);

	return true;
}