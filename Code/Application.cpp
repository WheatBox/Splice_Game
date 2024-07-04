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

#include <GLFW/glfw3.h>

void CApplication::Initialize(int, char **) {
	SetVSync(true);
	//SetVSync(false);
	//SetMaxFPS(60);

	Assets::LoadPermanentAssets();
	Texts::InitializeTexts(Texts::ELanguage::Chinese);
	//Texts::InitializeTexts(Texts::ELanguage::English);

	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->GetOrCreateComponent<CPhysicsWorldComponent>();
	}
	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->GetOrCreateComponent<CEditorComponent>();
	}

	//Frame::gRenderer->SetBackgroundColor(0x00004F);
	//Frame::gRenderer->SetBackgroundColor(0xFFFFFF);
	//Frame::gRenderer->SetBackgroundColor(0xDDDDDD);
	Frame::gRenderer->SetBackgroundColor(0xB1B1C1);

	//m_pSubWindow = glfwCreateWindow(640, 360, "Controller", NULL, NULL);
	// TODO
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