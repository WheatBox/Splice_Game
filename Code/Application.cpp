#include "Application.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameRender/Renderer.h>
#include <FrameCore/Camera.h>

#include "Components/PhysicsWorldComponent.h"
#include "Components/RigidbodyComponent.h"

#include "Components/Editor/EditorComponent.h"

#include "Assets.h"
#include "Texts.h"

void CApplication::Initialize(int, char **) {
	SetVSync(true);
	//SetVSync(false);
	//SetMaxFPS(60);

	Assets::LoadPermanentAssets();
	Texts::InitializeTexts(Texts::ELanguage::Chinese);
	//Texts::InitializeTexts(Texts::ELanguage::English);

	/*
	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		//Frame::CStaticSprite * pSprite = new Frame::CStaticSprite { "Assets/spr_test.bmp" };
		Frame::CAnimatedSprite * pSprite = new Frame::CAnimatedSprite { "Assets/StripTest.png", 4 };
		auto [offx, offy] = pSprite->GetSize();
		pSprite->SetOffset({ static_cast<float>(offx) * .5f, static_cast<float>(offy) * .5f });
		//CSpriteComponent * pSpriteComponent = pEntity->CreateComponent<CSpriteComponent>();
		//pSpriteComponent->SetSprite(pSprite);
		pEntity->CreateComponent<CTestComponent>();
		
		Frame::gCamera->SetZoom(.7f);
	}
	*/
	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->GetOrCreateComponent<CPhysicsWorldComponent>();
	}
	/*
	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0.f, 10.f);
		b2PolygonShape groundBox;
		groundBox.SetAsBox(50.f, 10.f);
		b2FixtureDef fixtureDef;
		fixtureDef.shape = & groundBox;
		fixtureDef.density = 0.f;
		auto * comp = pEntity->GetOrCreateComponent<CRigidbodyComponent>();
		comp->Physicalize(groundBodyDef, fixtureDef);
		comp->SetEnableRendering(true);
	}
	*/

	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->GetOrCreateComponent<CEditorComponent>();
	}

	//Frame::gRenderer->SetBackgroundColor(0x00004F);
	//Frame::gRenderer->SetBackgroundColor(0xFFFFFF);
	//Frame::gRenderer->SetBackgroundColor(0xDDDDDD);
	Frame::gRenderer->SetBackgroundColor(0xB1B1C1);
}

void CApplication::MainLoopPriority() {
	Frame::gCamera->SetViewSize(Frame::gCamera->GetWindowSize());
}

void CApplication::MainLoopLast() {
	for(const auto & entId : m_entitiesWillBeRemovedAtTheEndOfThisFrame) {
		Frame::gEntitySystem->RemoveEntity(entId);
	}
	m_entitiesWillBeRemovedAtTheEndOfThisFrame.clear();
}

CApplication * gApplication = new CApplication {};
b2World * gWorld = nullptr;