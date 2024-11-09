#include "ColliderComponent.h"

#include <FrameEntity/Entity.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include <cmath>

REGISTER_ENTITY_COMPONENT(CColliderComponent);

std::unordered_set<CColliderComponent *> CColliderComponent::s_colliders {};

std::list<CColliderComponent *> CColliderComponent::Collide() const {
	std::list<CColliderComponent *> resList;
	for(CColliderComponent * pCollider : s_colliders) {
		if(pCollider == this)
			continue;
		for(const SCollider & myCollider : m_colliders) {
			for(const SCollider & anotherCollider : pCollider->m_colliders) {
				Frame::Vec2 myPos = m_pEntity->GetPosition() + myCollider.offset;
				Frame::Vec2 pos = pCollider->GetEntity()->GetPosition() + anotherCollider.offset;
				Frame::Vec2 sizHalf = anotherCollider.size * .5f;
				if(std::abs(pos.x - myPos.x) <= sizHalf.x + myCollider.size.x * .5f && std::abs(pos.y - myPos.y) <= sizHalf.y + myCollider.size.y * .5f) {
					resList.push_back(pCollider);
				}
			}
		}
	}
	return resList;
}

void CColliderComponent::Render(const Frame::ColorRGB & color, float alpha) const {
	for(const SCollider & myCollider : m_colliders) {
		Frame::Vec2 myPos = m_pEntity->GetPosition() + myCollider.offset;
		Frame::Vec2 sizHalf = myCollider.size * .5f;
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(myPos - sizHalf, myPos + sizHalf, color, alpha);
	}
}
