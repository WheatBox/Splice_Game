#include "RigidbodyComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Application.h"

#include "Machine/DeviceComponent.h"

REGISTER_ENTITY_COMPONENT(, CRigidbodyComponent);

void CRigidbodyComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::Render:
	{
		if(!bRender) {
			break;
		}

		const int color = m_pBody->IsAwake() ? 0xFF7E00 : 0xB4B4B4;

		for(const auto & pFixture : m_fixtures) {
			b2Shape * pShape = pFixture->GetShape();
			switch(pShape->m_type) {
			case b2Shape::Type::e_polygon:
			{
				b2PolygonShape * pPolygonShape = reinterpret_cast<b2PolygonShape *>(pShape);
				
				for(int i = 2; i < pPolygonShape->m_count; i++) {
					Frame::Vec2 posAdd { m_pBody->GetPosition().x, m_pBody->GetPosition().y };
					Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[0].x, pPolygonShape->m_vertices[0].y }.Rotate(m_pBody->GetAngle())),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[i].x, pPolygonShape->m_vertices[i].y }.Rotate(m_pBody->GetAngle())),
						MeterToPixelVec2(posAdd + Frame::Vec2 { pPolygonShape->m_vertices[i - 1].x, pPolygonShape->m_vertices[i - 1].y }.Rotate(m_pBody->GetAngle())),
						color, .5f
					);
				}
				for(int i = 0; i < pPolygonShape->m_count; i++) {
					Frame::Vec2 posAdd { m_pBody->GetPosition().x, m_pBody->GetPosition().y };
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
			}
		}
	}
	break;
	}
}

void CRigidbodyComponent::Physicalize(const b2BodyDef & bodyDef, const b2FixtureDef & fixtureDef) {
	m_pBody = gWorld->CreateBody(& bodyDef);
	m_fixtures.push_back(m_pBody->CreateFixture(& fixtureDef));
}

void CRigidbodyComponent::Physicalize(const b2BodyDef & bodyDef, const std::vector<b2FixtureDef *> & fixtureDefs) {
	m_pBody = gWorld->CreateBody(& bodyDef);
	for(const auto & fixtureDef : fixtureDefs) {
		m_fixtures.push_back(m_pBody->CreateFixture(fixtureDef));
	}
}

void CRigidbodyComponent::Physicalize(const b2BodyDef & bodyDef, const std::vector<b2FixtureDef *> & fixtureDefs, const std::unordered_map<b2FixtureDef *, CDeviceComponent *> & map_fixtureDefDeviceComp) {
	m_pBody = gWorld->CreateBody(& bodyDef);
	for(const auto & fixtureDef : fixtureDefs) {
		b2Fixture * pFixture = m_pBody->CreateFixture(fixtureDef);
		m_fixtures.push_back(pFixture);
		if(auto it = map_fixtureDefDeviceComp.find(fixtureDef); it != map_fixtureDefDeviceComp.end()) {
			it->second->SetFixture(pFixture);
		}
	}
}

void CRigidbodyComponent::WeldWith(const CRigidbodyComponent * pRigidbodyComponent) {
	if(!pRigidbodyComponent) {
		return;
	}
	//b2Body * pAnotherBody = pRigidbodyComponent->GetBody();
	/*
	b2RevoluteJointDef rjd;
	//rjd.collideConnected = true;
	rjd.Initialize(pAnotherBody, m_pBody, m_pBody->GetWorldCenter());
	gWorld->CreateJoint(& rjd);
	rjd.Initialize(pAnotherBody, m_pBody, pAnotherBody->GetWorldCenter());
	gWorld->CreateJoint(& rjd);
	*/
	/*
	b2WeldJointDef wjd;
	const b2Vec2 anotherPos = pAnotherBody->GetPosition(), myPos = m_pBody->GetPosition();
	wjd.Initialize(pAnotherBody, m_pBody, anotherPos + b2Vec2 { (myPos.x - anotherPos.x) * .5f, (myPos.y - anotherPos.y) * .5f });
	gWorld->CreateJoint(& wjd);
	*/
}