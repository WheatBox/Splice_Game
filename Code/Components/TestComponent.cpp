#include "TestComponent.h"

#include <FrameEntity/EntitySystem.h>
#include <FrameCore/Globals.h>
#include <FrameInput/Input.h>
#include <FrameCore/Camera.h>

#include "../Application.h"
#include "RigidbodyComponent.h"

#include <cstdlib>
#include <ctime>

REGISTER_ENTITY_COMPONENT(, CTestComponent);

void CTestComponent::Initialize() {
	srand(static_cast<uint32>(time(NULL)));
	//m_pSpriteComponent = m_pEntity->GetOrCreateComponent<CSpriteComponent>();
}

Frame::EntityEvent::Flags CTestComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Update;
}

CRigidbodyComponent * p = nullptr;

void CTestComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	switch(event.flag) {
	case Frame::EntityEvent::EFlag::Update:
	{
		if(Frame::gInput->pMouse->GetPressed(Frame::eMBI_Left)) {
			if(auto pEnt = Frame::gEntitySystem->SpawnEntity()) {
				Frame::Vec2 spawnPos = PixelToMeterVec2(Frame::gCamera->WindowToScene(Frame::gInput->pMouse->GetPosition()));
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(spawnPos.x, spawnPos.y);
				bodyDef.angle = Frame::DegToRad(static_cast<float>(rand() % 360));
				b2PolygonShape dynamicBox;
				dynamicBox.SetAsBox(1.f, 1.f);
				b2FixtureDef fixtureDef;
				fixtureDef.shape = & dynamicBox;
				fixtureDef.density = 100.f;
				fixtureDef.friction = .3f;
				fixtureDef.restitution = .3f;
				auto * comp = pEnt->GetOrCreateComponent<CRigidbodyComponent>();
				comp->Physicalize(bodyDef, { fixtureDef });
				comp->SetEnableRendering(true);
				comp->WeldWith(p);
				p = comp;
			}
		}
		if(Frame::gInput->pMouse->GetPressed(Frame::eMBI_Right)) {
			if(1) {
				constexpr int siz = 2;
				CRigidbodyComponent * comps[siz * siz] = {};
				Frame::Vec2 spawnPos = PixelToMeterVec2(Frame::gCamera->WindowToScene(Frame::gInput->pMouse->GetPosition()));
				for(int i = 0; i < siz * siz; i++) {
					b2BodyDef bodyDef;
					bodyDef.type = b2_dynamicBody;
					bodyDef.position.Set(spawnPos.x + float(i % siz) * 2.2f, spawnPos.y + float(i / siz) * 2.2f);
					bodyDef.angle = 0.f;
					b2PolygonShape dynamicBox;
					dynamicBox.SetAsBox(1.f, 1.f);
					b2FixtureDef fixtureDef;
					fixtureDef.shape = & dynamicBox;
					fixtureDef.density = 100.f;
					fixtureDef.friction = .5f;
					fixtureDef.restitution = 0.f;
					auto pEnt = Frame::gEntitySystem->SpawnEntity();
					auto * comp = pEnt->GetOrCreateComponent<CRigidbodyComponent>();
					comp->Physicalize(bodyDef, { fixtureDef });
					comp->SetEnableRendering(true);
					comps[i] = comp;
				}
				comps[0]->WeldWith(comps[1]);
				comps[0]->WeldWith(comps[2]);
				comps[1]->WeldWith(comps[3]);
				comps[2]->WeldWith(comps[3]);
			}
		}
	}
	break;
	}
}