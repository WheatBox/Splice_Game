#include "RigidbodyComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Application.h"

#include "Machine/DeviceComponent.h"
#include "PhysicsWorldComponent.h"

REGISTER_ENTITY_COMPONENT(, CRigidbodyComponent);

void CRigidbodyComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Render:
	{
		if(!bRender || !m_pBody) {
			break;
		}

		const int color = m_pBody->IsAwake() ? 0xFF7E00 : 0xB4B4B4;

		for(const auto & pFixture : m_fixtures) {
			b2Shape * pShape = pFixture->GetShape();
			switch(pShape->m_type) {
			case b2Shape::Type::e_polygon:
			{
				b2PolygonShape * pPolygonShape = reinterpret_cast<b2PolygonShape *>(pShape);

				Frame::Vec2 posAdd { m_pBody->GetPosition().x, m_pBody->GetPosition().y };

				for(int i = 2; i < pPolygonShape->m_count; i++) {
					Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[0].x, pPolygonShape->m_vertices[0].y }.Rotate(m_pBody->GetAngle())),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[i].x, pPolygonShape->m_vertices[i].y }.Rotate(m_pBody->GetAngle())),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[i - 1].x, pPolygonShape->m_vertices[i - 1].y }.Rotate(m_pBody->GetAngle())),
						color, .5f
					);
				}
				for(int i = 0; i < pPolygonShape->m_count; i++) {
					b2Vec2 pos = pPolygonShape->m_vertices[i];
					b2Vec2 posPrev = (i == 0 ? pPolygonShape->m_vertices[pPolygonShape->m_count - 1] : pPolygonShape->m_vertices[i - 1]);
					Frame::gRenderer->pShapeRenderer->DrawLineBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { posPrev.x, posPrev.y }.Rotate(m_pBody->GetAngle())),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pos.x, pos.y }.Rotate(m_pBody->GetAngle())),
						color, 1.f
					);
				}
			}
			break;
			case b2Shape::Type::e_circle:
			{
				b2CircleShape * pCircleShape = reinterpret_cast<b2CircleShape *>(pShape);
				
				Frame::Vec2 posAdd = Frame::Vec2 { m_pBody->GetPosition().x, m_pBody->GetPosition().y } + Frame::Vec2 { pCircleShape->m_p.x, pCircleShape->m_p.y }.Rotate(m_pBody->GetAngle());
				constexpr int vertCount = 12;
				constexpr float angleAdd = Frame::DegToRad(360.f / static_cast<float>(vertCount));

				float angle = 2.f * angleAdd;
				b2Vec2 posPrev { std::cos(angleAdd) * pCircleShape->m_radius, std::sin(angleAdd) * pCircleShape->m_radius };
				b2Vec2 posFirst { pCircleShape->m_radius, 0.f };
				for(int i = 2; i < vertCount; i++) {
					b2Vec2 pos { std::cos(angle) * pCircleShape->m_radius, std::sin(angle) * pCircleShape->m_radius };
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
				posPrev = { std::cos(-angleAdd) * pCircleShape->m_radius, std::sin(-angleAdd) * pCircleShape->m_radius };
				for(int i = 0; i < vertCount; i++) {
					b2Vec2 pos { std::cos(angle) * pCircleShape->m_radius, std::sin(angle) * pCircleShape->m_radius };
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
	if(b2Body * pBody = m_pBody) {
		CPhysicsWorldComponent::s_physicalizeQueue.push(
			[pBody]() { gWorld->DestroyBody(pBody); }
		);
	}
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, b2FixtureDef fixtureDef) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, fixtureDef]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2Body * pBody = gWorld->CreateBody(& bodyDef)) {
						pComp->SetBody(pBody);
						pComp->GetFixtures().push_back(pBody->CreateFixture(& fixtureDef));
					}
				}
			}
		}
	);
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, std::vector<b2FixtureDef *> fixtureDefs) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, fixtureDefs]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2Body * pBody = gWorld->CreateBody(& bodyDef)) {
						pComp->SetBody(pBody);
						for(const auto & fixtureDef : fixtureDefs) {
							pComp->GetFixtures().push_back(pBody->CreateFixture(fixtureDef));
						}
					}
				}
			}
			CDeviceComponent::DestroyFixtureDefs(fixtureDefs);
		}
	);
}

void CRigidbodyComponent::Physicalize(b2BodyDef bodyDef, std::vector<b2FixtureDef *> fixtureDefs, std::unordered_map<b2FixtureDef *, CDeviceComponent *> map_fixtureDefDeviceComp) {
	Frame::EntityId myEntityId = m_pEntity->GetId();
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, bodyDef, fixtureDefs, map_fixtureDefDeviceComp]() {
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					if(b2Body * pBody = gWorld->CreateBody(& bodyDef)) {
						pComp->SetBody(pBody);
						for(const auto & fixtureDef : fixtureDefs) {
							b2Fixture * pFixture = pBody->CreateFixture(fixtureDef);
							pComp->GetFixtures().push_back(pFixture);

							if(auto it = map_fixtureDefDeviceComp.find(fixtureDef); it != map_fixtureDefDeviceComp.end()) {
								it->second->SetFixture(pFixture);
							}
						}
					}
				}
			}
			CDeviceComponent::DestroyFixtureDefs(fixtureDefs);
		}
	);
}

bool CRigidbodyComponent::CreateJointWith(Frame::EntityId entityId, std::function<b2JointDef * (b2Body *, b2Body *)> funcCreateJointDef, std::function<void (b2JointDef *)> funcDestroyJointDef) {
	Frame::EntityId myEntityId = m_pEntity->GetId();

	if(myEntityId == entityId) {
		return false;
	}
	
	CPhysicsWorldComponent::s_physicalizeQueue.push(
		[myEntityId, entityId, funcCreateJointDef, funcDestroyJointDef]() {
			b2Body * pMyBody = nullptr, * pAnotherBody = nullptr;
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(myEntityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					pMyBody = pComp->GetBody();
				}
			}
			if(Frame::CEntity * pEntity = Frame::gEntitySystem->GetEntity(entityId)) {
				if(CRigidbodyComponent * pComp = pEntity->GetComponent<CRigidbodyComponent>()) {
					pAnotherBody = pComp->GetBody();
				}
			}

			if(pMyBody && pAnotherBody) {
				auto pDef = funcCreateJointDef(pMyBody, pAnotherBody);
				gWorld->CreateJoint(pDef);
				funcDestroyJointDef(pDef);
			}
		}
	);

	return true;
}