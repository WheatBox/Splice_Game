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
#include "GUI/GUI.h"

#include <GLFW/glfw3.h>

void CApplication::Initialize() {
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
		CEditorComponent::s_pEditorComponent = pEntity->GetOrCreateComponent<CEditorComponent>();
	}

	//Frame::gRenderer->SetBackgroundColor(0x00004F);
	//Frame::gRenderer->SetBackgroundColor(0xFFFFFF);
	//Frame::gRenderer->SetBackgroundColor(0xDDDDDD);
	Frame::gRenderer->SetBackgroundColor(0xB1B1C1);

	m_cursors[eCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_cursors[eCursor_Ibeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_cursors[eCursor_Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	m_cursors[eCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	m_cursors[eCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
	m_cursors[eCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
	m_cursors[eCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
	m_cursors[eCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
	m_cursors[eCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
	m_cursors[eCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
	
	m_cursors[eCursor_Unknown] = m_cursors[eCursor_Arrow];
	
	SetCursor(eCursor_Arrow);

	//m_pSubWindow = glfwCreateWindow(640, 360, "Controller", NULL, NULL);
	// TODO
}

void CApplication::MainLoopPriority() {
	if(m_cursorWill == eCursor_Arrow) {
		SetCursor(eCursor_Arrow);
	}
	m_cursorWill = eCursor_Arrow;
	Frame::gCamera->SetViewSize(Frame::gCamera->GetWindowSize());
}

void CApplication::MainLoopLast() {
	for(const auto & entId : m_entitiesWillBeRemovedAtTheEndOfThisFrame) {
		Frame::gEntitySystem->RemoveEntity(entId);
	}
	m_entitiesWillBeRemovedAtTheEndOfThisFrame.clear();

	if(GUI::gCurrentGUI) {
		GUI::gCurrentGUI->Work();
	}
}

void CApplication::SetCursor(ECursor shape) {
	m_cursorWill = shape;
	if(shape == m_cursorCurr || !m_cursors[shape]) {
		return;
	}
	m_cursorCurr = shape;
	glfwSetCursor(m_pWindow, m_cursors[shape]);
}

CApplication * gApplication = new CApplication {};
b2World * gWorld = nullptr;