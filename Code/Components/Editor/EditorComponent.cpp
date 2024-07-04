#include "EditorComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameInput/Input.h>
#include <FrameRender/Renderer.h>
#include <FrameCore/Camera.h>
#include <FrameMath/Math.h>

#include "EditorDeviceComponent.h"
#include "../CameraComponent.h"
#include "../Machine/DeviceConnectorRendererComponent.h"
#include "../Machine/MachinePartComponent.h"
#include "../Machine/MachineComponent.h"
#include "../PhysicsWorldComponent.h"

#include "../../Depths.h"

REGISTER_ENTITY_COMPONENT(, CEditorComponent);

constexpr float __INTERFACESET_BUTTON_SIZE_HALF = 48.f;
constexpr float __INTERFACE_SIGN_WIDTH_HALF = 8.f;
constexpr float __INTERFACE_SIGN_HEIGHT_HALF = 32.f;
constexpr float __ERASER_BUTTON_SIZE_HALF = 40.f;
constexpr float __PIPE_BUTTON_SIZE_HALF = 14.f;

constexpr float __PENCIL_BUTTON_ALPHA = .7f;
constexpr float __INTERFACESET_BUTTON_ALPHA = __PENCIL_BUTTON_ALPHA * .5f;
constexpr float __ERASER_BUTTON_ALPHA = .5f;
constexpr float __CONTROLLER_BUTTON_ALPHA = .7f;
constexpr float __CONTROLLER_BUTTON_TEXT_ALPHA = .85f;
constexpr int __PENCIL_BUTTON_COLOR = 0xFF7400;
constexpr int __PENCIL_BUTTON_EDGE_COLOR = 0xFFBB4D;
constexpr int __ERASER_BUTTON_COLOR = 0x0074FF;
constexpr int __ERASER_BUTTON_EDGE_COLOR = 0x4DBBFF;
constexpr int __CONTROLLER_BUTTON_COLOR = 0xBD4400;
constexpr int __CONTROLLER_BUTTON_EDGE_COLOR = 0xBD7F1B;

constexpr int __GUI_BACKGROUND_COLOR = 0x1E1E1E;
constexpr int __GUI_BACKGROUND_EDGE_COLOR = 0xFFFFFF;
constexpr int __GUI_BUTTON_COLOR = 0x2F2F2F;
constexpr int __GUI_BUTTON_HIGHLIGHT_COLOR = 0x4D4D4D;
constexpr int __GUI_BUTTON_EDGE_COLOR = 0x4D4D4D;
constexpr int __GUI_BUTTON_TEXT_COLOR = 0xCCCCCC;

constexpr float __TOOLBAR_BUTTON_ICON_ALPHA = .75f;
constexpr float __TOOLBAR_BUTTON_ICON_SCALE = .5f;

constexpr float __PIPE_HIDE_ALPHA = .3f; // 管道工具插入模式，要插入的目标管道之外的其它管道的透明度
constexpr float __DEVICE_HIDE_ALPHA = .3f;

#define __DRAW_TEXT_BUTTON(center, sizehalf, highlight, text) { \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(center - sizehalf, center + sizehalf, highlight ? __GUI_BUTTON_HIGHLIGHT_COLOR : __GUI_BUTTON_COLOR, 1.f); \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(center - sizehalf, center + sizehalf, __GUI_BUTTON_EDGE_COLOR, 1.f, 1.f); \
	Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(text, center + Frame::Vec2 { .0f, -1.f }, Frame::ETextHAlign::Center, Frame::ETextVAlign::Middle, __GUI_BUTTON_TEXT_COLOR, 1.f); \
}

void CEditorComponent::Initialize() {
	m_pEntity->SetZDepth(Depths::Editor);

	SetWorking(true);

	Put({ 0.f }, IDeviceData::EType::Cabin, 0);

	m_pFont = new Frame::CFont { Assets::GetFontFilename(), 16.f };

	m_pCameraComponent = m_pEntity->CreateComponent<CCameraComponent>();
	if(m_pCameraComponent) {
		m_pCameraComponent->Initialize(
			[this]() {
				return Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Left) && m_tool == ETool::Hand || Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Middle);
			},
			[]() { return false; }
		);
		m_pCameraComponent->SetWorking(false);
	}

	m_pDeviceConnectorRendererEntity = Frame::gEntitySystem->SpawnEntity();
	if(m_pDeviceConnectorRendererEntity) {
		if(CDeviceConnectorRendererComponent * pDeviceConnectorRendererComponent = m_pDeviceConnectorRendererEntity->CreateComponent<CDeviceConnectorRendererComponent>()) {
			pDeviceConnectorRendererComponent->Initialize(& m_editorDeviceComponents);
		}
	}
}

void CEditorComponent::OnShutDown() {
	// TODO - 清理所有 SEditorPipeNode
	// 和 CEditorDeviceComponent
	// 和 CEditorDeviceComponent 的 CEntity

	delete m_pFont;

	if(m_pDeviceConnectorRendererEntity) {
		Frame::gEntitySystem->RemoveEntity(m_pDeviceConnectorRendererEntity->GetId());
	}
}

Frame::EntityEvent::Flags CEditorComponent::GetEventFlags() const {
	return Frame::EntityEvent::Update | Frame::EntityEvent::Render;
}

void CEditorComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {

	if(!m_bWorking) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::Update:
	{
		CameraControl();

		DrawBlockBackground();
	}
	break;
	case Frame::EntityEvent::Render:
	{
		const bool bMBLeftPressed = Frame::gInput->pMouse->GetPressed(Frame::eMBI_Left);
		const bool bMBLeftHolding = Frame::gInput->pMouse->GetHolding(Frame::eMBI_Left);
		const bool bMBLeftReleased = Frame::gInput->pMouse->GetReleased(Frame::eMBI_Left);
		const bool bMBRightPressed = Frame::gInput->pMouse->GetPressed(Frame::eMBI_Right);
		m_bMBLeftPressed = false;
		m_bMBLeftHolding = false;
		m_bMBLeftReleased = false;
		m_bMBRightPressed = false;

		Frame::gRenderer->pTextRenderer->SetFont(m_pFont);

		{
			const Frame::ColorRGB pipeColor = GetCurrentColorSet().pipe;
			for(size_t i = 0, siz = m_pipes.size(); i < siz; i++) {
				const auto & _pipeNodes = m_pipes[i];
				float alpha = (i == m_pipeInsertData.pipeIndex || m_pipeInsertData.pipeIndex == SIZE_MAX) ? 1.f : __PIPE_HIDE_ALPHA;
				if(m_tool == ETool::Controller && !m_toolControllerStuff.highlightEDComps.empty()) {
					alpha = m_toolControllerStuff.highlightPipeIndices.find(i) == m_toolControllerStuff.highlightPipeIndices.end() ? __DEVICE_HIDE_ALPHA : alpha;
				}
				DrawMyPipe(_pipeNodes, alpha);
			}
		}

		/* --------------------- HotKeys --------------------- */

		do {
			if(m_toolControllerStuff.pEDCompWaitingForKey) {
				break;
			}

			if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_1)) SwitchTool(ETool::Hand);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_2)) SwitchTool(ETool::Pencil);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_3)) SwitchTool(ETool::Eraser);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_4)) SwitchTool(ETool::Pipe);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_5)) SwitchTool(ETool::Swatches);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_6)) SwitchTool(ETool::Controller);

			if(m_tool == ETool::Pencil) {
				const bool bGoPrev = Frame::gInput->pKeyboard->GetPressed(Frame::eKI_Q);
				const bool bGoNext = Frame::gInput->pKeyboard->GetPressed(Frame::eKI_E);
				if((bGoPrev || bGoNext) && m_pencilDevice == IDeviceData::Unset) {
					SwitchPencilDevice(IDeviceData::Shell);
				} else if(bGoPrev) {
					IDeviceData::EType deviceTemp = static_cast<IDeviceData::EType>(m_pencilDevice - 1);
					if(deviceTemp < IDeviceData::Shell) deviceTemp = static_cast<IDeviceData::EType>(IDeviceData::END - 1);
					SwitchPencilDevice(deviceTemp);
				} else if(bGoNext) {
					IDeviceData::EType deviceTemp = static_cast<IDeviceData::EType>(m_pencilDevice + 1);
					if(deviceTemp >= IDeviceData::END) deviceTemp = IDeviceData::Shell;
					SwitchPencilDevice(deviceTemp);
				}
			} else if(m_tool == ETool::Pipe) {
				if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_Q)) {
					constexpr EPipeToolMode arr[] = { EPipeToolMode::Insert, EPipeToolMode::Pencil, EPipeToolMode::Eraser };
					SwitchPipeToolMode(arr[static_cast<int>(m_pipeToolMode)]);
				} else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_E)) {
					constexpr EPipeToolMode arr[] = { EPipeToolMode::Eraser, EPipeToolMode::Insert, EPipeToolMode::Pencil };
					SwitchPipeToolMode(arr[static_cast<int>(m_pipeToolMode)]);
				}
			}

		} while(false);

		/* ----------------------- Canvas ----------------------- */

		if(!m_bMouseOnGUI) {
			m_bMBLeftPressed = bMBLeftPressed;
			m_bMBLeftHolding = bMBLeftHolding;
			m_bMBLeftReleased = bMBLeftReleased;
			m_bMBRightPressed = bMBRightPressed;
		}

		if(m_tool == ETool::Pencil && m_pencilDevice != IDeviceData::EType::Unset) {
			Pencil();
		} else
		if(m_tool == ETool::Eraser) {
			Eraser();
		} else
		if(m_tool == ETool::Pipe) {
			switch(m_pipeToolMode) {
			case EPipeToolMode::Pencil:
				Pipe_PencilMode();
				break;
			case EPipeToolMode::Eraser:
				Pipe_EraserMode();
				break;
			case EPipeToolMode::Insert:
				Pipe_InsertMode();
				break;
			}
		} else
		if(m_tool == ETool::Controller) {
			Controller();
		}

		/* ----------------------- GUI ----------------------- */

		const float camZoomBeforeGUI = Frame::gCamera->GetZoom();
		const Frame::Vec2 camPosBeforeGUI = Frame::gCamera->GetPos();
		const float camRotBeforeGUI = Frame::gCamera->GetRotation();
		Frame::gCamera->SetZoom(1.f);
		const Frame::Vec2 viewSize = Frame::Vec2Cast(Frame::gCamera->GetViewSize());
		Frame::gCamera->SetPos(viewSize * .5f);
		Frame::gCamera->SetRotation(0.f);

		if(m_bMouseOnGUI) {
			m_bMBLeftPressed = bMBLeftPressed;
			m_bMBLeftHolding = bMBLeftHolding;
			m_bMBLeftReleased = bMBLeftReleased;
			m_bMBRightPressed = bMBRightPressed;
		}

		m_bMouseOnGUI = false;

		const Frame::Vec2 toolbarLeftTopPos = { 0.f };//Frame::gCamera->GetPos() - viewSize * .5f;
		const Frame::Vec2 toolbarRightBottomPos = toolbarLeftTopPos + Frame::Vec2 { toolbarWidth, viewSize.y };
		RenderAndProcessToolbar(toolbarLeftTopPos, toolbarRightBottomPos);

		Frame::Vec2 toolMenuLT = { toolbarRightBottomPos.x, toolbarLeftTopPos.y };
		switch(m_tool) {
		case ETool::Pencil:
			RenderAndProcessPencilMenu(toolMenuLT);
			break;
		case ETool::Pipe:
			RenderAndProcessPipeMenu(toolMenuLT);
			break;
		case ETool::Swatches:
			RenderAndProcessSwatchesMenu(toolMenuLT);
			break;
		case ETool::Controller:
			RenderAndProcessControllerMenu(toolMenuLT);
			ButtonEnd(toolbarLeftTopPos + viewSize);
			break;
		}

		if(m_mouseLabelText != Texts::EText::EMPTY) {
			Texts::DrawTextLabel(m_mouseLabelText, Frame::gInput->pMouse->GetPosition() + Frame::Vec2 { 16.f, 0.f });
			m_mouseLabelText = Texts::EText::EMPTY;
		}

		DrawOperationPrompt(toolbarRightBottomPos + Frame::Vec2 { GetToolMenuWidth(m_tool) + 8.f, -8.f });

		Frame::gCamera->SetZoom(camZoomBeforeGUI);
		Frame::gCamera->SetPos(camPosBeforeGUI);
		Frame::gCamera->SetRotation(camRotBeforeGUI);

		/* --------------------------------------------------- */
	}
	break;
	}
}

void CEditorComponent::SetWorking(bool b) {
	m_bWorking = b;
	if(CPhysicsWorldComponent::s_pPhysicsWorldComponent) {
		CPhysicsWorldComponent::s_pPhysicsWorldComponent->SetEditorWorking(b);
	}
	if(m_pDeviceConnectorRendererEntity) {
		if(auto pComp = m_pDeviceConnectorRendererEntity->GetComponent<CDeviceConnectorRendererComponent>()) {
			pComp->SetWorking(b);
		}
	}
	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->SetWorking(b);
	}
}

void CEditorComponent::RenderAndProcessToolbar(const Frame::Vec2 & leftTopPos, const Frame::Vec2 & rightBottomPos) {
	constexpr static ETool arrTools[] = { ETool::Hand, ETool::Pencil, ETool::Eraser, ETool::Pipe, ETool::Swatches, ETool::Controller };
	constexpr static Assets::EGUIStaticSprite arrSprs[] = {
		Assets::EGUIStaticSprite::Editor_tool_hand,
		Assets::EGUIStaticSprite::Editor_tool_pencil,
		Assets::EGUIStaticSprite::Editor_tool_eraser,
		Assets::EGUIStaticSprite::Editor_tool_pipe,
		Assets::EGUIStaticSprite::Editor_tool_swatches,
		Assets::EGUIStaticSprite::Editor_tool_controller
	};
	constexpr static Texts::EText arrTexts[] = {
		Texts::EText::EditorToolHand,
		Texts::EText::EditorToolPencil,
		Texts::EText::EditorToolEraser,
		Texts::EText::EditorToolPipe,
		Texts::EText::EditorToolSwatches,
		Texts::EText::EditorToolController
	};
	constexpr static size_t toolsNum = sizeof(arrTools) / sizeof(* arrTools);

	constexpr float buttonSize = toolbarWidth;

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(leftTopPos, rightBottomPos, __GUI_BACKGROUND_COLOR, 1.f);

	Frame::Vec2 buttonPos = leftTopPos + Frame::Vec2 { 0.f, buttonSize * 1.5f };
	for(size_t i = 0; i < toolsNum; i++) {
		Frame::Vec2 buttonLT = buttonPos, buttonRB = buttonPos + Frame::Vec2 { buttonSize };
		bool bMouseOn = Frame::PointInRectangle(mousePosInScene, buttonLT, buttonRB - 1.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(buttonLT, buttonRB, bMouseOn || arrTools[i] == m_tool ? __GUI_BUTTON_HIGHLIGHT_COLOR : __GUI_BUTTON_COLOR, 1.f);
		Frame::gRenderer->DrawSpriteAlphaBlended(Assets::GetStaticSprite(arrSprs[i])->GetImage(), buttonLT + buttonSize * .5f, __TOOLBAR_BUTTON_ICON_ALPHA, __TOOLBAR_BUTTON_ICON_SCALE, 0.f);
		if(bMouseOn) {
			SetMouseLabel(arrTexts[i]);
		}
		if(bMouseOn && m_bMBLeftPressed) {
			SwitchTool(arrTools[i]);
		}
		buttonPos.y += buttonSize;
	}

	if(float toolMenuWidth = GetToolMenuWidth(m_tool); toolMenuWidth > 0.f) { // Tool menu background
		Frame::Vec2 lt { rightBottomPos.x, leftTopPos.y };
		Frame::Vec2 rb { rightBottomPos.x + toolMenuWidth, rightBottomPos.y };
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __GUI_BACKGROUND_COLOR, 1.f);
		Frame::gRenderer->pShapeRenderer->DrawLineBlended({ rb.x, lt.y }, rb, __GUI_BACKGROUND_EDGE_COLOR, 1.f);
	}

	Frame::gRenderer->pShapeRenderer->DrawLineBlended({ rightBottomPos.x, leftTopPos.y }, rightBottomPos, __GUI_BACKGROUND_EDGE_COLOR, 1.f);
}

void CEditorComponent::RenderAndProcessPencilMenu(const Frame::Vec2 & leftTopPos) {
	constexpr float buttonSize = GetToolMenuWidth(ETool::Pencil);

	SColorSet colorSet = GetCurrentColorSet();

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	int posIndex = 0;

	// 只显示从 Shell 开始的装置
	for(IDeviceData::EType type = IDeviceData::Shell; type < IDeviceData::END; type = static_cast<IDeviceData::EType>(type + 1)) {
		bool bChoosing = m_pencilDevice == type;
		float spriteScale = 1.f;
		switch(type) {
		case IDeviceData::Shell:
		case IDeviceData::Engine: spriteScale = .8f; break;
		case IDeviceData::Propeller: spriteScale = .4f; break;
		case IDeviceData::JetPropeller: spriteScale = .5f; break;
		case IDeviceData::Joint: spriteScale = .8f; break;
		}
		Frame::Vec2 pos = leftTopPos + Frame::Vec2 { buttonSize * .5f, buttonSize * (static_cast<float>(posIndex) + .5f) };
		posIndex++;

		DrawDevicePreview(type, pos, 1.f, 0, spriteScale);

		float highlightAlpha = .0f;
		const Frame::Vec2 highlightHalfWH { buttonSize * .5f - 1.f };
		if(bChoosing) {
			highlightAlpha = .3f;
			goto Highlight;
		} else if(Frame::PointInRectangle(mousePosInScene, pos - highlightHalfWH, pos + highlightHalfWH)) {
			highlightAlpha = .15f;

			if(m_bMBLeftPressed) {
				SwitchPencilDevice(type);
			}

			goto Highlight;
		}

		continue;
	Highlight:
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos - highlightHalfWH, pos + highlightHalfWH, 0xFFFFFF, highlightAlpha);
	}
}

void CEditorComponent::RenderAndProcessPipeMenu(const Frame::Vec2 & leftTopPos) {
	constexpr static EPipeToolMode arrModes[] = { EPipeToolMode::Pencil, EPipeToolMode::Eraser, EPipeToolMode::Insert };
	constexpr static Assets::EGUIStaticSprite arrSprs[] = {
		Assets::EGUIStaticSprite::Editor_tool_pipe_mode_pencil,
		Assets::EGUIStaticSprite::Editor_tool_pipe_mode_eraser,
		Assets::EGUIStaticSprite::Editor_tool_pipe_mode_insert,
	};
	constexpr static Texts::EText arrTexts[] = {
		Texts::EText::EditorToolPipe_ModePencil,
		Texts::EText::EditorToolPipe_ModeEraser,
		Texts::EText::EditorToolPipe_ModeInsert,
	};
	constexpr static size_t modesNum = sizeof(arrModes) / sizeof(* arrModes);

	constexpr float menuWidth = GetToolMenuWidth(ETool::Pipe);
	constexpr float buttonSize = menuWidth - 1;

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	Frame::Vec2 buttonPos = leftTopPos + Frame::Vec2 { 0.f, buttonSize * 1.5f };
	for(size_t i = 0; i < modesNum; i++) {
		Frame::Vec2 buttonLT = buttonPos, buttonRB = buttonPos + Frame::Vec2 { buttonSize };
		bool bMouseOn = Frame::PointInRectangle(mousePosInScene, buttonLT, buttonRB);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(buttonLT, buttonRB, bMouseOn || arrModes[i] == m_pipeToolMode ? __GUI_BUTTON_HIGHLIGHT_COLOR : __GUI_BUTTON_COLOR, 1.f);
		Frame::gRenderer->DrawSpriteAlphaBlended(Assets::GetStaticSprite(arrSprs[i])->GetImage(), buttonLT + buttonSize * .5f, __TOOLBAR_BUTTON_ICON_ALPHA, __TOOLBAR_BUTTON_ICON_SCALE, 0.f);
		if(bMouseOn) {
			SetMouseLabel(arrTexts[i]);
		}
		if(bMouseOn && m_bMBLeftPressed) {
			SwitchPipeToolMode(arrModes[i]);
		}
		buttonPos.y += buttonSize;
	}
}

void CEditorComponent::RenderAndProcessSwatchesMenu(const Frame::Vec2 & leftTopPos) {
	constexpr float menuWidth = GetToolMenuWidth(ETool::Swatches);

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	const float colorBlockSize = 32.f;
	const float colorBlockSpacing = 4.f;
	const float colorSetEdge = 8.f;
	
	Frame::Vec2 colorSetLT = leftTopPos;
	for(size_t i = 0ull, len = m_colorSets.size(); i < len; i++) {
		auto & colorSet = m_colorSets[i];

		Frame::Vec2 colorBlockLT = colorSetLT + colorSetEdge;

#define __DrawColorBlock(memberVarInColorSet) \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorBlockLT, colorBlockLT + colorBlockSize, colorSet.memberVarInColorSet, 1.f); \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorBlockLT, colorBlockLT + colorBlockSize, 0xFFFFFF, 1.f, 1.f); \
	colorBlockLT.x += colorBlockSize + colorBlockSpacing;

		__DrawColorBlock(color1);
		__DrawColorBlock(color2);
		__DrawColorBlock(connector);
		__DrawColorBlock(pipe);

#undef __DrawColorBlock

		const Frame::Vec2 colorSetRB = colorSetLT + Frame::Vec2 { menuWidth - 1.f, colorBlockSize + colorSetEdge * 2.f };
		bool bMouseOn = Frame::PointInRectangle(mousePosInScene, colorSetLT, colorSetRB);
		if(i == m_currColorSetIndex || bMouseOn) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorSetLT + 1.f, colorSetRB - 1.f, 0xFFFFFF, i == m_currColorSetIndex ? 1.f : .5f, 2.f);
		}
		if(bMouseOn && m_bMBLeftPressed) {
			m_bMBLeftPressed = false;

			m_currColorSetIndex = i;
			SynchCurrentColorSet();
			UpdateDevicesColor();
		}

		colorSetLT.y += colorBlockSize + colorSetEdge * 2.f;
	}

	/* ----------------------------- Color Editor ----------------------------- */

	const Frame::Vec2 colorEditorSize { 380.f, 256.f };

	const Frame::Vec2 dragAreaLT { leftTopPos.x + menuWidth, leftTopPos.y };
	const Frame::Vec2 dragAreaRB = Frame::Vec2Cast<float>(Frame::gCamera->GetViewSize());

	m_colorEditorMenuDragger.WorkPart1(mousePosInScene, colorEditorSize, dragAreaLT, dragAreaRB);
	if(m_colorEditorMenuDragger.IsDragging()) {
		m_bMBLeftHolding = m_bMBLeftPressed = false;
	}

	const Frame::Vec2 & colorEditorLT = m_colorEditorMenuDragger.GetLeftTop();
	const Frame::Vec2 colorEditorRB = colorEditorLT + colorEditorSize;

	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorEditorLT, colorEditorRB, __GUI_BACKGROUND_COLOR, 1.f);
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorEditorLT, colorEditorRB, __GUI_BACKGROUND_EDGE_COLOR, 1.f, 1.f);

	constexpr int colorNum = 4;
	Frame::ColorRGB * colorSetEditingColors[colorNum] = { & m_colorSetEditing.color1, & m_colorSetEditing.color2, & m_colorSetEditing.connector, & m_colorSetEditing.pipe };
	for(int i = 0; i < colorNum; i++) {
		Frame::Vec2 lt = colorEditorLT + Frame::Vec2 { (colorBlockSize + 16.f) * static_cast<float>(i), 0.f } + 32.f;
		Frame::Vec2 rb = lt + colorBlockSize;
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, * colorSetEditingColors[i], 1.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, 1.f, 1.f);

		if(m_swatchesColorEditingIndex == i) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, 1.f, 4.f);
		}

		if(Frame::PointInRectangle(mousePosInScene, lt, rb)) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, .5f, 4.f);
			if(m_bMBLeftPressed) {
				m_bMBLeftPressed = false;
				m_swatchesColorEditingIndex = i;
			}
		}
	}

	Frame::ColorRGB & colorEditing = * colorSetEditingColors[m_swatchesColorEditingIndex];

#define __DRAW_COLOR_BAR(rgbPart, index, title, _isR, _isG, _isB) { \
		constexpr float barHeight = 12.f; \
		const Frame::Vec2 lt = colorEditorLT + Frame::Vec2 { 16.f, 96.f + index * 40.f }; \
		const Frame::Vec2 rb = lt + Frame::Vec2 { 224.f, barHeight }; \
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, Frame::ColorRGB(colorEditing.rgbPart * _isR, colorEditing.rgbPart * _isG, colorEditing.rgbPart * _isB), 1.f); \
		if(m_bMBLeftPressed && Frame::PointInRectangle(mousePosInScene, lt, rb)) { \
			m_bMBLeftHolding = m_bMBLeftPressed = false; \
			colorEditing.rgbPart = static_cast<uint8>(Frame::Clamp((mousePosInScene.x - lt.x) / (rb.x - lt.x), 0.f, 1.f) * 255.f); \
		} \
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, 1.f, 1.f); \
\
		const Frame::Vec2 pointerCenter = { lt.x + (rb.x - lt.x) * (static_cast<float>(colorEditing.rgbPart) / 255.f), lt.y + (rb.y - lt.y) * .5f }; \
\
		const float buttonYCenter = pointerCenter.y; \
		const Frame::Vec2 buttonSizeHalf { 16.f, 16.f }; \
		const Frame::Vec2 decButtonPos { rb.x + 32.f, buttonYCenter }; \
		const Frame::Vec2 incButtonPos { decButtonPos.x + 34.f, buttonYCenter }; \
		bool mouseOnDec = false; \
		bool mouseOnInc = false; \
		if(Frame::PointInRectangle(mousePosInScene, decButtonPos - buttonSizeHalf, decButtonPos + buttonSizeHalf)) { \
			mouseOnDec = true; \
			if(m_bMBLeftHolding && colorEditing.rgbPart > 0) { \
				m_bMBLeftHolding = m_bMBLeftPressed = false; \
				colorEditing.rgbPart--; \
			} \
		} \
		if(Frame::PointInRectangle(mousePosInScene, incButtonPos - buttonSizeHalf, incButtonPos + buttonSizeHalf)) { \
			mouseOnInc = true; \
			if(m_bMBLeftHolding && colorEditing.rgbPart < 255) { \
				m_bMBLeftHolding = m_bMBLeftPressed = false; \
				colorEditing.rgbPart++; \
			} \
		} \
\
		const Frame::Vec2 pointerLVecHalf { 0.f, barHeight * .5f}; \
		Frame::gRenderer->pShapeRenderer->DrawLineBlended(pointerCenter - pointerLVecHalf, pointerCenter + pointerLVecHalf, 0xFFFFFF, 1.f, 3.f); \
\
		__DRAW_TEXT_BUTTON(decButtonPos, buttonSizeHalf, mouseOnDec, "[-]"); \
		__DRAW_TEXT_BUTTON(incButtonPos, buttonSizeHalf, mouseOnInc, "[+]"); \
		Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(std::string { title } + std::to_string(colorEditing.rgbPart), { incButtonPos.x + buttonSizeHalf.x, incButtonPos.y }, Frame::ETextHAlign::Left, Frame::ETextVAlign::Middle, 0xFFFFFF, 1.f); \
	}

	__DRAW_COLOR_BAR(r, 0.f, " R: ", 1, 0, 0);
	__DRAW_COLOR_BAR(g, 1.f, " G: ", 0, 1, 0);
	__DRAW_COLOR_BAR(b, 2.f, " B: ", 0, 0, 1);

#undef __DRAW_COLOR_BAR

	{
		Frame::Vec2 btResetSizeHalf { 24.f, 12.f };
		Frame::Vec2 btApplySizeHalf = btResetSizeHalf;
		if(m_pFont) {
			btResetSizeHalf.x = 8.f + .5f * m_pFont->TextWidth(Texts::GetText(Texts::Reset), 0.f);
			btApplySizeHalf.x = 8.f + .5f * m_pFont->TextWidth(Texts::GetText(Texts::Apply), 0.f);
		}
		Frame::Vec2 btResetPos = colorEditorRB - btResetSizeHalf - 12.f;
		Frame::Vec2 btApplyPos = btResetPos - Frame::Vec2 { btResetSizeHalf.x + btApplySizeHalf.x + 16.f, 0.f };
		bool mouseOnReset = false;
		bool mouseOnApply = false;
		if(Frame::PointInRectangle(mousePosInScene, btResetPos - btResetSizeHalf, btResetPos + btResetSizeHalf)) {
			mouseOnReset = true;
			if(m_bMBLeftPressed) {
				m_bMBLeftHolding = m_bMBLeftPressed = false;
				SynchCurrentColorSet();
			}
		}
		if(Frame::PointInRectangle(mousePosInScene, btApplyPos - btApplySizeHalf, btApplyPos + btApplySizeHalf)) {
			mouseOnApply = true;
			if(m_bMBLeftPressed) {
				m_bMBLeftHolding = m_bMBLeftPressed = false;
				ApplyCurrentColorSet();
			}
		}
		__DRAW_TEXT_BUTTON(btResetPos, btResetSizeHalf, mouseOnReset, Texts::GetText(Texts::Reset));
		__DRAW_TEXT_BUTTON(btApplyPos, btApplySizeHalf, mouseOnApply, Texts::GetText(Texts::Apply));
	}

	char szColorCode[8];
	sprintf_s(szColorCode, 8, "#%02X%02X%02X", colorEditing.r, colorEditing.g, colorEditing.b);
	Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(szColorCode, { colorEditorLT.x + 32.f, colorEditorRB.y - 32.f }, Frame::ETextHAlign::Left, Frame::ETextVAlign::Middle, 0xFFFFFF, 1.f);

	m_colorEditorMenuDragger.WorkPart2(m_bMBLeftPressed, m_bMBLeftReleased, mousePosInScene, colorEditorSize);
	if(m_colorEditorMenuDragger.IsDragging()) {
		m_bMouseOnGUI = true;
	}
}

void CEditorComponent::RenderAndProcessControllerMenu(const Frame::Vec2 & leftTopPos) {
	constexpr float menuWidth = GetToolMenuWidth(ETool::Controller);

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	/* ----------------------- 底盘 ----------------------- */

	const Frame::Vec2 controllerSize = Vec2Cast(Frame::Vec2i { m_controllerEditing.gridWidth, m_controllerEditing.gridHeight } * Controller::gridCellSize);

	const Frame::Vec2 controllerLT = m_controllerMenuDragger.GetLeftTop();
	const Frame::Vec2 controllerRB = controllerLT + controllerSize;
	
	m_controllerMenuDragger.WorkPart1(mousePosInScene, controllerSize, { leftTopPos.x + menuWidth, leftTopPos.y }, Frame::Vec2Cast<float>(Frame::gCamera->GetViewSize()));
	if(m_controllerMenuDragger.IsDragging()) {
		m_bMBLeftPressed = m_bMBLeftHolding = false;
	}

	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(controllerLT, controllerRB, 0x4F4F4F, 1.f);
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(controllerLT, controllerRB, __GUI_BACKGROUND_EDGE_COLOR, 1.f, 1.f);

	/* ----------------------- 拖动现有元件 ----------------------- */
	
	for(auto & pElem : m_controllerEditing.elements) {
		if(m_bMBLeftPressed && !m_pDraggingControllerElement) {
			if(auto [lt, rb] = Controller::GetElementAABBRealPos(pElem, controllerLT); Frame::PointInRectangle(mousePosInScene, lt, rb)) {
				m_bMBLeftPressed = m_bMBLeftHolding = false;
				m_pDraggingControllerElement = pElem;
				m_draggingontrollerElementPosRelativeToMouse = Controller::GetElementRealPos(pElem, controllerLT) - mousePosInScene;
				break;
			}
		}
	}

	/* ----------------------- 绘制现有元件 ----------------------- */

	for(auto & pElem : m_controllerEditing.elements) {
		Controller::DrawPreview(pElem->element, Controller::GetElementRealPos(pElem, controllerLT),
			pElem == m_pDraggingControllerElement ? .3f : 1.f, 1.f);
	}

	/* ----------------------- 元件列表 ----------------------- */

	int posIndex = 0;
	for(Controller::EElement elem = Controller::EElement::Button; elem < Controller::EElement::END; elem = static_cast<Controller::EElement>(static_cast<int>(elem) + 1)) {
		Frame::Vec2 pos = leftTopPos + Frame::Vec2 { menuWidth * .5f, menuWidth * (static_cast<float>(posIndex) + .5f) };
		posIndex++;

		Controller::DrawPreview(elem, pos, 1.f, 1.f);

		if(const Frame::Vec2 highlightHalfWH { menuWidth * .5f - 1.f }; Frame::PointInRectangle(mousePosInScene, pos - highlightHalfWH, pos + highlightHalfWH)) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos - highlightHalfWH, pos + highlightHalfWH, 0xFFFFFF, .15f);

			if(m_bMBLeftPressed && !m_pDraggingControllerElement) {
				m_bMBLeftPressed = m_bMBLeftHolding = false;
				m_pDraggingControllerElement = std::make_shared<Controller::SButtonElement>();
				m_draggingontrollerElementPosRelativeToMouse = 0.f;
			}
		}
	}

	/* ----------------------- 拖动并放置元件 ----------------------- */

	if(m_pDraggingControllerElement) {
		Frame::Vec2i posDraggingTo;
		{
			Frame::Vec2i temp = Frame::Vec2Cast<int>(mousePosInScene + m_draggingontrollerElementPosRelativeToMouse - controllerLT) + Controller::gridCellSize / 2;
			posDraggingTo = {
				temp.x / Controller::gridCellSize,
				temp.y / Controller::gridCellSize
			};
		}

		bool bCanPut = false;
		bool bWillRemove = false;

		auto [aabbLT, aabbRB] = m_pDraggingControllerElement->GetAABB();
		aabbLT = aabbLT - m_pDraggingControllerElement->pos + posDraggingTo;
		aabbRB = aabbRB - m_pDraggingControllerElement->pos + posDraggingTo;
		if(Controller::AABBIntersect(aabbLT, aabbRB, 0, { m_controllerEditing.gridWidth, m_controllerEditing.gridHeight })) {
			bCanPut = true;
			if(aabbLT.x < 0 || aabbLT.y < 0 || aabbRB.x > m_controllerEditing.gridWidth || aabbRB.y > m_controllerEditing.gridHeight) {
				bCanPut = false;
			}
			else
			for(auto & pElementOnController : m_controllerEditing.elements) {
				if(pElementOnController == m_pDraggingControllerElement) {
					continue;
				}
				if(Controller::AABBIntersect(* pElementOnController, aabbLT, aabbRB)) {
					bCanPut = false;
					break;
				}
			}

			const Frame::Vec2 pos = Controller::GetElementRealPos(posDraggingTo, controllerLT);

			Controller::DrawPreview(m_pDraggingControllerElement->element, pos, 1.f, 1.f);
			Controller::DrawAABB(m_pDraggingControllerElement->element, pos, bCanPut ? 0x00FF00 : 0xFF0000, .5f);
		} else {
			const Frame::Vec2 previewPos = mousePosInScene + m_draggingontrollerElementPosRelativeToMouse;

			if(mousePosInScene.x < leftTopPos.x + menuWidth - 8.f) {
				bWillRemove = true;
			}
			
			Controller::DrawPreview(m_pDraggingControllerElement->element, previewPos, bWillRemove ? .5f : 1.f, 1.f);

			if(bWillRemove) {
				auto [linePos1, linePos2] = Controller::GetElementAABBRealPos(m_pDraggingControllerElement, 0.f);
				const Frame::Vec2 elementRealPos = Controller::GetElementRealPos(m_pDraggingControllerElement, 0.f);
				linePos1 = linePos1 - elementRealPos + previewPos;
				linePos2 = linePos2 - elementRealPos + previewPos;
				Frame::gRenderer->pShapeRenderer->DrawLineBlended(linePos1, linePos2, 0x000000, 1.f, 8.f);
			}
		}

		if(m_bMBLeftReleased) {
			if(bCanPut) {
				m_pDraggingControllerElement->pos = posDraggingTo;
				m_controllerEditing.elements.insert(m_pDraggingControllerElement);
			}

			if(bWillRemove) {
				if(auto it = m_controllerEditing.elements.find(m_pDraggingControllerElement); it != m_controllerEditing.elements.end()) {
					m_controllerEditing.elements.erase(it);
				}
			}

			m_pDraggingControllerElement = nullptr;
		}
	}

	/* ---------------------------------------------------- */

	m_controllerMenuDragger.WorkPart2(m_bMBLeftPressed && !m_pDraggingControllerElement, m_bMBLeftReleased, mousePosInScene, controllerSize);
	if(m_controllerMenuDragger.IsDragging()) {
		m_bMouseOnGUI = true;
	}
}

void CEditorComponent::Pencil() {
	bool bPutFinished = false;
	const Frame::Vec2 mousePos = GetMousePosInScene();
	const SInterfaceSet * pInterfaceSetMouseOn = nullptr;
	const SInterface * pInterfaceMouseOn = nullptr;
	for(auto & interfaceSet : m_interfaces) {
		const Frame::Vec2 lt = interfaceSet.pos - __INTERFACESET_BUTTON_SIZE_HALF;
		const Frame::Vec2 rb = interfaceSet.pos + __INTERFACESET_BUTTON_SIZE_HALF;
		if(!m_bMouseOnGUI && !pInterfaceMouseOn && Frame::PointInRectangle(mousePos, lt, rb)) {
			pInterfaceSetMouseOn = & interfaceSet;

			float minRad = 999999.f;
			for(auto & interface : interfaceSet.interfaces) {
				if(float rad = std::abs(GetDirPosAdd(interface.directionIndex).IncludedAngle(interfaceSet.pos - GetMousePosInScene())); rad < minRad) {
					minRad = rad;
					pInterfaceMouseOn = & interface;
				}
			}

			if(pInterfaceMouseOn) {
				if(m_pInterfaceMouseOn != pInterfaceMouseOn) {
					m_pInterfaceMouseOn = pInterfaceMouseOn;
					RefreshInterfaceCanPut(* pInterfaceMouseOn);
				}

				//DrawDevicePreview(m_pencilDevice, GetWillPutPos(* pInterfaceMouseOn), __PENCIL_BUTTON_ALPHA, interface.directionIndex, 1.f, 0xFF0000, !m_bInterfaceCanPut);
				DrawDevicePreview(m_pencilDevice, GetWillPutPos(* pInterfaceMouseOn), __PENCIL_BUTTON_ALPHA, pInterfaceMouseOn->directionIndex, 1.f, m_bInterfaceCanPut ? 0xFFFFFF : 0xFF0000);
			}
		}
		if(pInterfaceSetMouseOn != & interfaceSet) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_COLOR, __INTERFACESET_BUTTON_ALPHA);
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_EDGE_COLOR, __INTERFACESET_BUTTON_ALPHA, 1.f);
		}
	}

	for(auto & interfaceSet : m_interfaces) {
		for(auto & interface : interfaceSet.interfaces) {
			if(interface.pEditorDeviceComponent == nullptr) {
				continue;
			}
			//Frame::Vec2 entityPos = interface.pEditorDeviceComponent->GetEntity()->GetPosition();
		
			Frame::Vec2 p1 { __INTERFACE_SIGN_WIDTH_HALF, __INTERFACE_SIGN_HEIGHT_HALF };
			Frame::Vec2 p2 { __INTERFACE_SIGN_WIDTH_HALF, -__INTERFACE_SIGN_HEIGHT_HALF };
			Frame::Vec2 p3 { -__INTERFACE_SIGN_WIDTH_HALF, 0.f };
			Frame::Rotate2DVectorsDegree(-GetDegreeByDirIndex(interface.directionIndex), { & p1, & p2, & p3 });
			p1 += interface.pos;
			p2 += interface.pos;
			p3 += interface.pos;
			Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(p1, p2, p3, __PENCIL_BUTTON_COLOR, __PENCIL_BUTTON_ALPHA);
			Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(p1, p2, p3, __PENCIL_BUTTON_EDGE_COLOR, __PENCIL_BUTTON_ALPHA, 1.f);
			if(bPutFinished) {
				continue;
			}

			const bool bMouseOn = pInterfaceMouseOn == & interface;

			if(m_bMBLeftHolding && bMouseOn && m_bInterfaceCanPut) {
				if(CEditorDeviceComponent * pEDComp = Put(interface)) {

					//////////// 检查新放置的装置的接口与此前已有接口的距离，若距离过近则进行连接
					std::vector<SInterface> v;
					pEDComp->GetAvailableInterfaces(& v);
					for(SInterface & newerInterface : v) {
						for(const SInterfaceSet & olderInterfaceSet : m_interfaces) {
							for(const SInterface & olderInterface : olderInterfaceSet.interfaces) {
								if(& olderInterface == & interface
									|| PointDistance(newerInterface.pos, olderInterface.pos) > 32.f)
									continue;
								if(newerInterface.pEditorDeviceComponent->ConnectWith(olderInterface.pEditorDeviceComponent, newerInterface.directionIndex)) {
									break;
								}
							}
						}
					}
					//////////////////////////////////

					bPutFinished = true;
					// 直接这么写的话，在放置装置的时候，按钮会闪烁一下，所以换了种写法（见下文中的 注1）
					//ClearAvailableInterfaces();
					//FindAvailableInterfaces();
					//break;
				}
			}
		}
	}

	if(bPutFinished) { // 上文提到的 注1
		ClearAvailableInterfaces();
		FindAvailableInterfaces();
	}

	if(!pInterfaceMouseOn) {
		m_pInterfaceMouseOn = nullptr;
		m_bInterfaceCanPut = false;
	}
}

void CEditorComponent::Eraser() {
	if(m_editorDeviceComponents.size() <= 1) {
		return;
	}
	auto itWillBeErase = m_editorDeviceComponents.end();
	for(auto it = m_editorDeviceComponents.begin(); it != m_editorDeviceComponents.end(); it++) {
		if((* it)->GetDeviceType() == IDeviceData::EType::Cabin) {
			continue;
		}

		const Frame::Vec2 pos = (* it)->GetEntity()->GetPosition();
		const Frame::Vec2 lt = pos - __ERASER_BUTTON_SIZE_HALF, rb = pos + __ERASER_BUTTON_SIZE_HALF;
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __ERASER_BUTTON_COLOR, __ERASER_BUTTON_ALPHA);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __ERASER_BUTTON_EDGE_COLOR, __ERASER_BUTTON_ALPHA, 1.f);

		if(itWillBeErase != m_editorDeviceComponents.end()) {
			continue;
		}
		if(m_bMBLeftHolding && Frame::PointInRectangle(GetMousePosInScene(), lt, rb)) {
			itWillBeErase = it;
		}
	}
	if(itWillBeErase != m_editorDeviceComponents.end()) {

		auto pEDComp = * itWillBeErase;

		/* ----------------- 擦除附着的管道 ----------------- */

		std::vector<SPipeInterface> pipeInterfacesTemp;
		pEDComp->GetPipeInterfaces(& pipeInterfacesTemp);
		for(const auto & _pipeInterface : pipeInterfacesTemp) {
			bool hasPipe = false;
			for(size_t i = 0, siz = m_pipes.size(); i < siz; i++) {
				for(const auto & _pPipeNode : m_pipes[i]) {
					if(_pPipeNode->pDevice == pEDComp && _pPipeNode->dirIndexForDevice == _pipeInterface.directionIndex) {
						hasPipe = true;
						ErasePipeNode(i, _pPipeNode);
						break;
					}
				}
				if(hasPipe) {
					break;
				}
			}
		}

		/* ------------------------------------------------ */

		Frame::gEntitySystem->RemoveEntity(pEDComp->GetEntity()->GetId());
		m_editorDeviceComponents.erase(itWillBeErase);
	}
}

void CEditorComponent::Pipe_PencilMode() {
	const size_t minIndexCanBeEdited = m_pipeInsertData.pipeIndex == SIZE_MAX ? 0 : m_pipeInsertData.pipeEditingMinIndex;

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();
	const Frame::ColorRGB pipeColor = GetCurrentColorSet().pipe;

	const SPipeInterface * pPipeInterfaceMouseOn = nullptr;
	{
		size_t i = 0;
		bool bSelectOver = false;
		for(auto & interface : m_pipeInterfaces) {
			if(m_pipeInterfaceSelectingIndex == i
				|| (IsSelectingPipeInterface() && GetSelectingPipeInterface().pEditorDeviceComponent == m_pipeInterfaces[i].pEditorDeviceComponent)
				|| m_pipeInsertData.devicesThatHasAlreadyConnected.find(m_pipeInterfaces[i].pEditorDeviceComponent) != m_pipeInsertData.devicesThatHasAlreadyConnected.end()
				) {
				i++;
				continue;
			}
			const Frame::Vec2 pos = interface.pos;
			constexpr float sizeHalf = __PIPE_BUTTON_SIZE_HALF;
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos - sizeHalf, pos + sizeHalf, __PENCIL_BUTTON_COLOR, __PENCIL_BUTTON_ALPHA);
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos - sizeHalf, pos + sizeHalf, __PENCIL_BUTTON_EDGE_COLOR, __PENCIL_BUTTON_ALPHA, 1.f);
			if(!bSelectOver && Frame::PointInRectangle(mousePosInScene, pos - sizeHalf, pos + sizeHalf)) {
				pPipeInterfaceMouseOn = & interface;

				if(m_bMBLeftPressed && !IsSelectingPipeInterface() && m_pipeInsertData.pipeIndex == SIZE_MAX) {
					m_pipeInterfaceSelectingIndex = i;

					m_pipeNodesEditing.push_back(CreatePipeNodeByInterface(interface));

					m_bMBLeftPressed = false;
					m_bMBRightPressed = false;

					bSelectOver = true;
				}
			}
			i++;
		}

		if(bSelectOver) {
			FindAvailablePipeInterfacesMachinePart(GetSelectingPipeInterface());
			return;
		}
	}

	if(!IsSelectingPipeInterface() && m_pipeInsertData.pipeIndex == SIZE_MAX) {
		return;
	}

	if(m_bMBRightPressed) { // 右键单击以撤销正在进行中的编辑

		if(m_pipeNodesEditing.size() <= minIndexCanBeEdited) {
			goto _RightClickToUndo_Over;
		}
		else
			if(m_pipeNodesEditing.size() == minIndexCanBeEdited + 1) {
				if(m_pipeInsertData.pipeIndex != SIZE_MAX) {
					UndoNewCrossOfPipeNodesInserting();
					GiveBackPipeNodesEditingToInsertMode();
					return;
				}
				CancelPipeNodesEditing();
				FindAvailablePipeInterfaces();
				goto _RightClickToUndo_Over;
			}

		UndoPipeNodesEditing();

	_RightClickToUndo_Over:

		DrawMyPipe(m_pipeNodesEditing);

		return;
	}

	{
		int dirIndex = 0;
		Frame::Vec2 pos {};
		Frame::Vec2 vPosMouse {};
		/*
		const bool bToInterface = m_pipeNodesEditing.size() == 0;
		if(bToInterface) {
		pos = m_pipeInterfaces[m_pipeInterfaceSelectingIndex].pos;
		vPosMouse = mousePosInScene - pos;
		dirIndex = m_pipeInterfaces[m_pipeInterfaceSelectingIndex].directionIndex;
		} else */
		if(m_pipeNodesEditing.size() > minIndexCanBeEdited) {
			pos = m_pipeNodesEditing.back()->pos;

			if(pPipeInterfaceMouseOn) {
				vPosMouse = pPipeInterfaceMouseOn->pos - pos;
			} else {
				vPosMouse = mousePosInScene - pos;
			}

			if(m_pipeNodesEditing.size() <= minIndexCanBeEdited + 1) {
				if(m_pipeInsertData.pipeIndex != SIZE_MAX) {
					dirIndex = GetDirIndex(vPosMouse);
					if(const SEditorPipeNode * p = m_pipeNodesEditing.back(); p->nodes[dirIndex]) {
						float nearestRad = Frame::pi_f;
						for(int i = 0; i < 4; i++) {
							if(!p->nodes[i]) {
								if(const float rad = GetDirPosAdd(i).IncludedAngle(vPosMouse); rad < nearestRad) {
									dirIndex = i;
									nearestRad = rad;
								}
							}
						}
					}
				} else {
					dirIndex = GetSelectingPipeInterface().directionIndex;
				}
			} else {
				dirIndex = GetDirIndex(vPosMouse);
				if(const int pipeRevDirIndex = GetDirIndex(m_pipeNodesEditing[m_pipeNodesEditing.size() - 2ull]->pos - pos); pipeRevDirIndex == dirIndex) {
					const float degree = (vPosMouse.x == 0.f && vPosMouse.y == 0.f) ? 0.f : -vPosMouse.Degree();
					if(const int dirIndexTemp = GetDirIndexByDegree(degree + 45.f); dirIndexTemp != pipeRevDirIndex) {
						dirIndex = dirIndexTemp;
					} else {
						dirIndex = GetDirIndexByDegree(degree - 45.f);
					}
				}
			}
		}

		const Frame::Vec2 dirPosAdd = GetDirPosAdd(dirIndex);
		Frame::Vec2 vPosDest { dirPosAdd.x * vPosMouse.x > 0.f ? vPosMouse.x : 0.f, dirPosAdd.y * vPosMouse.y > 0.f ? vPosMouse.y : 0.f };
		if(constexpr float minLen = PIPE_CROSS_SIZE; vPosDest.Length() < minLen) {
			vPosDest = dirPosAdd * minLen;
		}
		const Frame::Vec2 posDest = pos + vPosDest;

		DrawMyPipe(m_pipeNodesEditing);
		DrawPipeSingleLine({ std::min(pos.x, posDest.x), std::max(pos.y, posDest.y) }, { std::max(pos.x, posDest.x), std::min(pos.y, posDest.y) }, pipeColor, .5f);
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_interface_color)->GetImage(), posDest, pipeColor, .5f);
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_interface)->GetImage(), posDest, 0xFFFFFF, .5f);

		if(m_bMBLeftPressed) {

			if(m_pipeNodesEditing.size() >= minIndexCanBeEdited + 2) {
				// 如果要添加的新节点和上两个节点在同一条线上，清除上一个节点，避免三个节点处在同一条线段上
				if(SEditorPipeNode * pPrev = m_pipeNodesEditing.back(); pPrev->nodes[GetRevDirIndex(dirIndex)]) {
					int count = 0;
					for(int i = 0; i < 4; i++) {
						count += m_pipeNodesEditing.back()->nodes[i] != nullptr;
					}
					if(count < 2) {
						delete pPrev;
						m_pipeNodesEditing.pop_back();
					}
				}
			}

			SEditorPipeNode * pPipeNode = new SEditorPipeNode { posDest };
			//if(!bToInterface) {
			SEditorPipeNode * pAnother = m_pipeNodesEditing.back();
			pPipeNode->nodes[GetRevDirIndex(dirIndex)] = pAnother;
			pAnother->nodes[dirIndex] = pPipeNode;
			//}
			m_pipeNodesEditing.push_back(pPipeNode);
			if(pPipeInterfaceMouseOn && std::abs(pPipeInterfaceMouseOn->pos.x - posDest.x) <= 4.f && std::abs(pPipeInterfaceMouseOn->pos.y - posDest.y) <= 4.f) {

				BindPipeNodeWithEditorDeviceComponent(pPipeNode, pPipeInterfaceMouseOn->pEditorDeviceComponent);
				pPipeNode->dirIndexForDevice = pPipeInterfaceMouseOn->directionIndex;

				if(m_pipeInsertData.pipeIndex != SIZE_MAX) {
					GiveBackPipeNodesEditingToInsertMode();
					return;
				} else {
					MovePipeNodesEditingToPipeNodes();

					FindAvailablePipeInterfaces();
				}
			}
		}
	}

	return;
}

void CEditorComponent::Pipe_EraserMode() {
	size_t pipeNodesIndex = SIZE_MAX;

	SEditorPipeNode * pPipeNodeChosen = nullptr;

	{
		for(size_t i = 0, siz = m_pipes.size(); i < siz; i++) {
			for(auto & _pPipeNode : m_pipes[i]) {
				if(_pPipeNode->pDevice) {
					const Frame::Vec2 lt = _pPipeNode->pos - __PIPE_BUTTON_SIZE_HALF;
					const Frame::Vec2 rb = _pPipeNode->pos + __PIPE_BUTTON_SIZE_HALF;
					Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __ERASER_BUTTON_COLOR, __ERASER_BUTTON_ALPHA);
					Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __ERASER_BUTTON_EDGE_COLOR, __ERASER_BUTTON_ALPHA, 1.f);

					if(m_bMBLeftHolding && Frame::PointInRectangle(GetMousePosInScene(), lt, rb)) {
						pipeNodesIndex = i;
						pPipeNodeChosen = _pPipeNode;
						//break;
					}
				}
			}
			//if(pipeNodesIndex != SIZE_MAX) {
			//break;
			//}
		}
	}

	if(pipeNodesIndex == SIZE_MAX) {
		return;
	}

	ErasePipeNode(pipeNodesIndex, pPipeNodeChosen);
}

void CEditorComponent::Pipe_InsertMode() {
	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	for(size_t i = 0, siz = m_pipes.size(); i < siz; i++) {
		if(m_pipeInsertData.pipeIndex != SIZE_MAX) {
			break;
		}
		const auto & _pPipeNodes = m_pipes[i];
		for(auto & _pPipeNode : _pPipeNodes) {
			if(_pPipeNode->pDevice) {
				const Frame::Vec2 lt = _pPipeNode->pos - __PIPE_BUTTON_SIZE_HALF;
				const Frame::Vec2 rb = _pPipeNode->pos + __PIPE_BUTTON_SIZE_HALF;
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_COLOR, __PENCIL_BUTTON_ALPHA);
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_EDGE_COLOR, __PENCIL_BUTTON_ALPHA, 1.f);

				if(m_bMBLeftPressed && Frame::PointInRectangle(GetMousePosInScene(), lt, rb)) {
					m_bMBLeftPressed = false;
					m_bMBRightPressed = false;
					m_pipeInsertData.pipeIndex = i;
					break;
				}
			}
		}
	}

	if(m_pipeInsertData.pipeIndex == SIZE_MAX) {
		return;
	}

	if(m_bMBRightPressed) {
		m_pipeInsertData.pipeIndex = SIZE_MAX;
		return;
	}

	const Frame::ColorRGB pipeColor = GetCurrentColorSet().pipe;

	auto & pipeNodes = m_pipes[m_pipeInsertData.pipeIndex];
	for(size_t ind = 0, siz = pipeNodes.size(); ind < siz; ind++) {
		size_t i = siz - ind - 1;
		SEditorPipeNode * pPipeNode = pipeNodes[i];
		for(int j = 0; j < 2; j++) {
			SEditorPipeNode * pPipeNodeAnother = pPipeNode->nodes[j];
			if(pPipeNodeAnother == nullptr) {
				continue;
			}
			const Frame::Vec2 lineDiffHalf = (pPipeNodeAnother->pos - pPipeNode->pos) * .5f;
			const Frame::Vec2 lineDiffHalfAbs { std::abs(lineDiffHalf.x), std::abs(lineDiffHalf.y) };
			const Frame::Vec2 lineMid = pPipeNode->pos + lineDiffHalf;
			const Frame::Vec2 areaHalf = lineDiffHalfAbs + PIPE_CROSS_SIZE * .5f;

			bool mouseOnCrossAnother = Frame::PointInRectangle(pPipeNodeAnother->pos, mousePosInScene - PIPE_CROSS_SIZE, mousePosInScene + PIPE_CROSS_SIZE);
			bool mouseOnCross = Frame::PointInRectangle(pPipeNode->pos, mousePosInScene - PIPE_CROSS_SIZE, mousePosInScene + PIPE_CROSS_SIZE);
			{
				int _count = 0, _countAnother = 0;
				for(int _i = 0; _i < 4; _i++) {
					_countAnother += pPipeNodeAnother->nodes[_i] != nullptr;
					_count += pPipeNode->nodes[_i] != nullptr;
				}
				mouseOnCrossAnother = _countAnother == 4 ? false : mouseOnCrossAnother;
				mouseOnCross = _count == 4 ? false : mouseOnCross;
			}
			const bool mouseOnCrossOrAnotherCross = mouseOnCross || mouseOnCrossAnother;
			bool crossWhichMouseOnIsOnDevice = (mouseOnCross && pPipeNode->pDevice) || (mouseOnCrossAnother && pPipeNodeAnother->pDevice);
			if(!crossWhichMouseOnIsOnDevice && (mouseOnCrossOrAnotherCross || Frame::PointInRectangle(mousePosInScene, lineMid + areaHalf, lineMid - areaHalf))) {
				Frame::Vec2 pos {};
				if(mouseOnCrossAnother) {
					pos = pPipeNodeAnother->pos;
				} else if(mouseOnCross) {
					pos = pPipeNode->pos;
				} else {
					pos = lineMid + lineDiffHalfAbs.GetNormalized() * (mousePosInScene - lineMid);
				}
				Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_interface_color)->GetImage(), pos, pipeColor, .5f);
				Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_interface)->GetImage(), pos, 0xFFFFFF, .5f);

				if(m_bMBLeftPressed) {
					if(mouseOnCrossAnother) {
						pipeNodes.push_back(pPipeNodeAnother);
						for(auto _it = pipeNodes.begin(); _it != pipeNodes.end(); _it++) {
							if(* _it == pPipeNodeAnother) {
								pipeNodes.erase(_it);
								break;
							}
						}
					} else if(mouseOnCross) {
						pipeNodes.push_back(pPipeNode);
						for(auto _it = pipeNodes.begin(); _it != pipeNodes.end(); _it++) {
							if(* _it == pPipeNode) {
								pipeNodes.erase(_it);
								break;
							}
						}
					}
					SEditorPipeNode * _pNode = mouseOnCrossOrAnotherCross ? pipeNodes.back() : new SEditorPipeNode { pos };

					if(!mouseOnCrossOrAnotherCross) {
						pPipeNodeAnother->nodes[GetRevDirIndex(j)] = _pNode;
						_pNode->nodes[j] = pPipeNodeAnother;

						pPipeNode->nodes[j] = _pNode;
						_pNode->nodes[GetRevDirIndex(j)] = pPipeNode;
					}

					CancelPipeNodesEditing();
					m_pipeNodesEditing.swap(m_pipes[m_pipeInsertData.pipeIndex]);
					m_pipeInsertData.isNewCross = !mouseOnCrossOrAnotherCross;
					if(m_pipeInsertData.isNewCross) {
						m_pipeNodesEditing.push_back(_pNode);
					}
					m_pipeInsertData.pipeEditingMinIndex = m_pipeNodesEditing.size() - 1;

					for(const auto & _pPipeNodeEditing : m_pipeNodesEditing) {
						if(_pPipeNodeEditing->pDevice) {
							m_pipeInsertData.devicesThatHasAlreadyConnected.insert(_pPipeNodeEditing->pDevice);
						}
					}

					m_pipeToolMode = EPipeToolMode::Pencil;

					for(auto & _pPipeNode : m_pipeNodesEditing) {
						if(_pPipeNode->pDevice) {
							FindAvailablePipeInterfacesMachinePart(_pPipeNode->pDevice);
							break;
						}
					}
				}

				ind = siz;
				break;
			}
		}
	}
}

void CEditorComponent::Controller() {
	static Frame::CFont * pFont = new Frame::CFont { Assets::GetFontFilename(), 36.f };

	Frame::CFont * pFontBefore = Frame::gRenderer->pTextRenderer->GetFont();

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	const float buttonSizeHalf = GetDevicePixelSize(IDeviceData::Engine).x * .4f;

	bool bMouseHasAlreadyOn = false;
	for(auto & pEDComp : m_toolControllerStuff.engineEDComps) {
		const Frame::Vec2 pos = pEDComp->GetEntity()->GetPosition();
		const Frame::Vec2 lt = pos - buttonSizeHalf, rb = pos + buttonSizeHalf;
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __CONTROLLER_BUTTON_COLOR, std::min(pEDComp->GetAlpha(), __CONTROLLER_BUTTON_ALPHA));
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __CONTROLLER_BUTTON_EDGE_COLOR, std::min(pEDComp->GetAlpha(), __CONTROLLER_BUTTON_ALPHA), 1.f);

		if(auto keyId = pEDComp->GetKeyId(); keyId != Frame::EKeyId::eKI_Unknown || m_toolControllerStuff.pEDCompWaitingForKey == pEDComp) {
			UnicodeString str;
			if(m_toolControllerStuff.pEDCompWaitingForKey == pEDComp) {
				str = Frame::UTF8Utils::ToUnicode("?");
			} else {
				str = Frame::UTF8Utils::ToUnicode("[") + GetKeyName(keyId) + Frame::UTF8Utils::ToUnicode("]");
			}
			Frame::gRenderer->pTextRenderer->SetFont(str.length() <= 3 ? pFont : m_pFont);
			Frame::gRenderer->pTextRenderer->DrawTextAutoWrapAlignBlended(str, pos, buttonSizeHalf * 2.f, Frame::ETextHAlign::Center, Frame::ETextVAlign::Middle, 0xFFFFFF, std::min(pEDComp->GetAlpha(), __CONTROLLER_BUTTON_TEXT_ALPHA));
		}

		const bool bMouseOn = Frame::PointInRectangle(mousePosInScene, lt, rb);

		if(!bMouseHasAlreadyOn && bMouseOn && m_toolControllerStuff.highlightEDComps.empty()) {
			bMouseHasAlreadyOn = true;

			for(auto & _pEDComp : m_editorDeviceComponents) {
				_pEDComp->SetAlpha(__DEVICE_HIDE_ALPHA);
			}

			std::function<void(CEditorDeviceComponent *)> recursion = [& recursion, this](CEditorDeviceComponent * pEDComp) {
				if(m_toolControllerStuff.highlightEDComps.find(pEDComp) != m_toolControllerStuff.highlightEDComps.end()) {
					return;
				}
				pEDComp->SetAlpha(1.f);
				m_toolControllerStuff.highlightEDComps.insert(pEDComp);

				std::unordered_set<SEditorPipeNode *> nodes;
				for(auto & pPipeNode : pEDComp->m_pipeNodes) {
					PipeRecursion(& nodes, pPipeNode, 0);
				}
				for(auto & pPipeNode : nodes) {
					if(!pPipeNode->pDevice || pEDComp == pPipeNode->pDevice) {
						continue;
					}
					recursion(pPipeNode->pDevice);
				}
				};
			recursion(pEDComp);

			// 能跑就行（
			for(const auto & _pEDComp : m_toolControllerStuff.highlightEDComps) {
				for(const auto & _nodesFinding : _pEDComp->m_pipeNodes) {
					for(size_t _pipeIndex = 0, _siz = m_pipes.size(); _pipeIndex < _siz; _pipeIndex++) {
						for(const auto & _pipeNode : m_pipes[_pipeIndex]) {
							if(_pipeNode == _nodesFinding) {
								m_toolControllerStuff.highlightPipeIndices.insert(_pipeIndex);
								_pipeIndex = _siz; // break 掉用 _pipeIndex 的那层 for
								break;
							}
						}
					}
				}
			}
		} else if(bMouseOn && !m_toolControllerStuff.highlightEDComps.empty()) {
			bMouseHasAlreadyOn = true;
		}

		if(bMouseOn && m_bMBLeftPressed) {
			m_toolControllerStuff.pEDCompWaitingForKey = m_toolControllerStuff.pEDCompWaitingForKey == pEDComp ? nullptr : pEDComp;
		}
	}

	if(m_bMBRightPressed) {
		m_toolControllerStuff.pEDCompWaitingForKey = nullptr;
	}

	if(m_toolControllerStuff.pEDCompWaitingForKey) {
		if(Frame::EKeyId keyIdPressed = GetAnyKeyPressed(); keyIdPressed != Frame::EKeyId::eKI_Unknown) {
			if(keyIdPressed == Frame::EKeyId::eKI_Escape) {
				m_toolControllerStuff.pEDCompWaitingForKey->SetKeyId(Frame::EKeyId::eKI_Unknown);
			} else {
				m_toolControllerStuff.pEDCompWaitingForKey->SetKeyId(keyIdPressed);
			}
			m_toolControllerStuff.pEDCompWaitingForKey = nullptr;
		}
	}

	if(!bMouseHasAlreadyOn && !m_toolControllerStuff.highlightEDComps.empty()) {
		for(auto & _pEDComp : m_editorDeviceComponents) {
			_pEDComp->SetAlpha(1.f);
		}
		m_toolControllerStuff.highlightEDComps.clear();
		m_toolControllerStuff.highlightPipeIndices.clear();
	}

	Frame::gRenderer->pTextRenderer->SetFont(pFontBefore);
}

void CEditorComponent::SwitchTool(ETool tool) {
	if(m_tool == ETool::Pencil) {
		ClearAvailableInterfaces();
	} else if(m_tool == ETool::Controller) {
		ControllerEnd();
	}

	m_tool = tool;

	if(m_tool == ETool::Pencil) {
		FindAvailableInterfaces();
	} else if(m_tool == ETool::Swatches) {
		SynchCurrentColorSet();
	} else if(m_tool == ETool::Controller) {
		ControllerBegin();
	}

	PipeToolCleanUp();
}

void CEditorComponent::UpdateDevicesColor() {
	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->UpdateColor(GetCurrentColorSet());
	}
}

void CEditorComponent::ControllerBegin() {
	for(auto & pEDComp : m_editorDeviceComponents) {
		if(pEDComp->GetDeviceType() == IDeviceData::Engine) {
			m_toolControllerStuff.engineEDComps.insert(pEDComp);
		}
	}
}

void CEditorComponent::ControllerEnd() {
	m_toolControllerStuff = SToolControllerStuff {};

	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->SetAlpha(1.f);
	}
}

void CEditorComponent::FindAvailableInterfaces() {
	for(auto & pEDComp : m_editorDeviceComponents) {
		std::vector<SInterface> interfaces;
		pEDComp->GetAvailableInterfaces(& interfaces);

		for(SInterface & interface : interfaces) {
			bool bNeedsToCreateANewInterfaceSet = true;
			for(SInterfaceSet & existingInterfaceSet : m_interfaces) {
				if(Frame::PointInRectangle(interface.pos + GetDirPosAdd(interface.directionIndex) * CONNECTOR_LENGTH, existingInterfaceSet.pos - __INTERFACESET_BUTTON_SIZE_HALF, existingInterfaceSet.pos + __INTERFACESET_BUTTON_SIZE_HALF)) {
					existingInterfaceSet.interfaces.push_back(interface);
					bNeedsToCreateANewInterfaceSet = false;
					break;
				}
			}
			if(bNeedsToCreateANewInterfaceSet) {
				m_interfaces.push_back({ { interface }, interface.pos + GetDirPosAdd(interface.directionIndex) * (__INTERFACESET_BUTTON_SIZE_HALF + CONNECTOR_HALF_LENGTH) });
			}
		}
	}
}

void CEditorComponent::ClearAvailableInterfaces() {
	m_pInterfaceMouseOn = nullptr;
	m_interfaces.clear();
}

void CEditorComponent::RefreshInterfaceCanPut(const SInterface & interfaceMouseOn) {
	if(Frame::CEntity * pEnt = Frame::gEntitySystem->SpawnEntity()) {
		if(CColliderComponent * pComp = pEnt->CreateComponent<CColliderComponent>()) {
			CEditorDeviceComponent::GetEditorDeviceColliders(pComp, m_pencilDevice, interfaceMouseOn.directionIndex);
			pComp->GetEntity()->SetPosition(GetWillPutPos(interfaceMouseOn));
			m_bInterfaceCanPut = pComp->Collide().empty();
		}
		Frame::gEntitySystem->RemoveEntity(pEnt->GetId());
	}
}

Frame::Vec2 CEditorComponent::GetWillPutPos(const SInterface & interface) const {
	IDeviceData::EType interfaceDevice = interface.pEditorDeviceComponent->GetDeviceType();
	int interfaceDeviceDirIndex = interface.pEditorDeviceComponent->GetDirIndex();

	return interface.pEditorDeviceComponent->GetEntity()->GetPosition()

		+ GetRectangleEdgePosByDirIndex(
			GetDevicePixelSize(interfaceDevice) + CONNECTOR_LENGTH * 2.f,
			interfaceDeviceDirIndex,
			interface.directionIndex
		)
		+ GetDeviceInterfaceBias(interfaceDevice, interfaceDeviceDirIndex, interface.directionIndex, 0.f)

		+ GetRectangleEdgePosByDirIndex(
			GetDevicePixelSize(m_pencilDevice),
			interface.directionIndex,
			interface.directionIndex
		);
}

CEditorDeviceComponent * CEditorComponent::Put(const CEditorComponent::SInterface & interface) {
	Frame::Vec2 putPos = GetWillPutPos(interface);
	if(CEditorDeviceComponent * pEDComp = Put(putPos, interface.directionIndex)) {
		interface.pEditorDeviceComponent->ConnectWith(pEDComp, interface.directionIndex);
		//if(interface.pEditorDeviceComponent->GetDeviceType() == IDeviceData::EType::Engine && pEDComp->GetDeviceType() == IDeviceData::EType::Engine) {
		// TODO or not - 当放置两个相邻的引擎时自动连接二者的管道
		//}
		return pEDComp;
	}
	return nullptr;
}

CEditorDeviceComponent * CEditorComponent::Put(const Frame::Vec2 & pos, IDeviceData::EType type, int dirIndex) {
	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->SetPosition(pos);
		CEditorDeviceComponent * pEDComp = pEntity->CreateComponent<CEditorDeviceComponent>();
		if(pEDComp->Initialize(this, type, dirIndex)) {
			m_editorDeviceComponents.insert(pEDComp);
			pEDComp->UpdateColor(GetCurrentColorSet());
			return pEDComp;
		} else {
			Frame::gEntitySystem->RemoveEntity(pEntity->GetId());
		}
	}
	return nullptr;
}

void CEditorComponent::GetAvailablePipeInterfaces(std::vector<SPipeInterface> * outToPushBack, CEditorDeviceComponent * pEDComp) const {
	std::vector<SPipeInterface> pipeInterfacesTemp;
	pEDComp->GetPipeInterfaces(& pipeInterfacesTemp);
	for(const auto & _pipeInterface : pipeInterfacesTemp) {
		bool hasPipe = false;
		for(const auto & _pipe : m_pipes) {
			for(const auto & _pPipeNode : _pipe) {
				if(_pPipeNode->pDevice == pEDComp && _pPipeNode->dirIndexForDevice == _pipeInterface.directionIndex) {
					hasPipe = true;
					break;
				}
			}
			if(hasPipe) {
				break;
			}
		}
		if(!hasPipe) {
			outToPushBack->push_back(_pipeInterface);
		}
	}
}

void CEditorComponent::FindAvailablePipeInterfaces() {
	DeselectPipeInterface();
	m_pipeInterfaces.clear();
	for(auto & pEDComp : m_editorDeviceComponents) {
		GetAvailablePipeInterfaces(& m_pipeInterfaces, pEDComp);
	}
}

// 注意该函数的参数不要在未来优化的时候给改成引用传参了！
void CEditorComponent::FindAvailablePipeInterfacesMachinePart(SPipeInterface pipeInterface) {
	// 因为这一步会改变 m_pipeInterfaces
	FindAvailablePipeInterfacesMachinePart(pipeInterface.pEditorDeviceComponent);

	size_t i = 0;
	for(auto & pipeInterfaceAfter : m_pipeInterfaces) {
		if(pipeInterfaceAfter.pos == pipeInterface.pos) {
			m_pipeInterfaceSelectingIndex = i;
			break;
		}
		i++;
	}
}

void CEditorComponent::FindAvailablePipeInterfacesMachinePart(CEditorDeviceComponent * _pEDComp) {
	if(!_pEDComp) {
		return;
	}

	DeselectPipeInterface();
	m_pipeInterfaces.clear();

	std::unordered_set<CEditorDeviceComponent *> machinePartEDComps;
	RecursiveMachinePartEditorDevices(& machinePartEDComps, _pEDComp);
	for(auto & pEDComp : machinePartEDComps) {
		GetAvailablePipeInterfaces(& m_pipeInterfaces, pEDComp);
	}
}

SEditorPipeNode * CEditorComponent::CreatePipeNodeByInterface(const SPipeInterface & interface) const {
	SEditorPipeNode * pPipeNode = new SEditorPipeNode { interface.pos };
	BindPipeNodeWithEditorDeviceComponent(pPipeNode, interface.pEditorDeviceComponent);
	pPipeNode->dirIndexForDevice = interface.directionIndex;
	return pPipeNode;
}

void CEditorComponent::ErasePipeNode(size_t pipeIndex, SEditorPipeNode * pipeNode) {
	std::unordered_set<SEditorPipeNode *> nodes;
	PipeRecursion(& nodes, pipeNode, 1);

	m_pipes[pipeIndex].erase(
		std::remove_if(m_pipes[pipeIndex].begin(), m_pipes[pipeIndex].end(), [& nodes, this](SEditorPipeNode * pNode) {
			if(nodes.find(pNode) != nodes.end()) {
				DestroyPipeNode(pNode);
				return true;
			}
			return false;
			})
		, m_pipes[pipeIndex].end()
	);

	if(m_pipes[pipeIndex].size() == 0) {
		m_pipes.erase(m_pipes.begin() + pipeIndex);
	}
}

void CEditorComponent::BindPipeNodeWithEditorDeviceComponent(SEditorPipeNode * pPipeNode, CEditorDeviceComponent * pEDComp) const {
	if(pPipeNode && pEDComp) {
		pEDComp->m_pipeNodes.insert(pPipeNode);
		pPipeNode->pDevice = pEDComp;
	}
}

void CEditorComponent::UnbindPipeNodeWithEditorDeviceComponent(SEditorPipeNode * pPipeNode) const {
	if(pPipeNode && pPipeNode->pDevice) {
		pPipeNode->pDevice->m_pipeNodes.erase(pPipeNode);
	}
}

void CEditorComponent::CameraControl() {
	static Frame::Vec2 mousePosPressed {};

	const Frame::Vec2 mouseCurrPos = GetMousePosInScene();
	const float camZoomRev = 1.f / Frame::gCamera->GetZoom();
	const Frame::Vec2 leftTopPos = Frame::gCamera->GetPos() - Frame::Vec2Cast(Frame::gCamera->GetViewSize()) * .5f * camZoomRev;
	
	if(mouseCurrPos.x <= leftTopPos.x + (toolbarWidth + GetToolMenuWidth(m_tool)) * camZoomRev) {
		mousePosPressed = mouseCurrPos;
		m_bMouseOnGUI = true;
		//return;
	}
	if(m_bMouseOnGUI) { // 因为其它地方也会控制 m_bMouseOnGUI 的值，所以就把这个 return 从上面单独拿出来了
		return;
	}

	if(m_pCameraComponent) {
		m_pCameraComponent->CameraControl();
	}
}

void CEditorComponent::DrawMyPipe(const std::vector<SEditorPipeNode *> & pipe, float alpha) const {
	DrawPipe<SEditorPipeNode>(pipe, m_pEntity->GetPosition(), GetCurrentColorSet().pipe, alpha, m_pEntity->GetRotation());
}

void CEditorComponent::DrawDevicePreview(IDeviceData::EType type, const Frame::Vec2 & pos, float alpha, int dirIndex, float scale, Frame::ColorRGB customColor, bool useCustomColor) const {

	const float rot = -GetDegreeByDirIndex(dirIndex);

	SColorSet colorSet = GetCurrentColorSet();
	Frame::ColorRGB baseColor = 0xFFFFFF;
	if(useCustomColor) {
		colorSet.color1 = customColor;
		colorSet.color2 = customColor;
		baseColor = customColor;
	}

#define __DrawDeviceSprite(_Assets_EDeviceStaticSprite, _color) Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::_Assets_EDeviceStaticSprite)->GetImage(), pos, _color, alpha, scale, rot);
#define __DrawDeviceSprite_ByPart(_Assets_EDeviceStaticSpritePart, _color) Frame::gRenderer->DrawSpriteBlended(Assets::GetDeviceStaticSprite(type, Assets::EDeviceStaticSpritePart::_Assets_EDeviceStaticSpritePart)->GetImage(), pos, _color, alpha, scale, rot);

	if(type == IDeviceData::JetPropeller) {
		__DrawDeviceSprite(jet_propeller_bottom, colorSet.color1);
	} else if(type == IDeviceData::Joint) {
		__DrawDeviceSprite(joint_bottom, colorSet.color2);
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::joint_color)->GetImage(), pos, colorSet.color1, alpha, scale, rot + 180.f);
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::joint)->GetImage(), pos, baseColor, alpha, scale, rot + 180.f);
	}

	if(type != IDeviceData::Joint) {
		__DrawDeviceSprite_ByPart(color1, colorSet.color1);
		__DrawDeviceSprite_ByPart(color2, colorSet.color2);
		__DrawDeviceSprite_ByPart(basic, baseColor);
	}

	if(type == IDeviceData::Propeller) {
		DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::propeller_blade_color)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.RotateDegree(rot) * scale, colorSet.color2, alpha, rot + 30.f, dirIndex % 2 ? Frame::Vec2 { scale, .3f * scale } : Frame::Vec2 { .3f * scale, scale }, 0.f);
		DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::propeller_blade)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.RotateDegree(rot) * scale, baseColor, alpha, rot + 30.f, dirIndex % 2 ? Frame::Vec2 { scale, .3f * scale } : Frame::Vec2 { .3f * scale, scale }, 0.f);
		__DrawDeviceSprite(propeller_top_color, colorSet.color1);
		__DrawDeviceSprite(propeller_top, baseColor);
	} else if(type == IDeviceData::JetPropeller) {
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::jet_propeller_needle)->GetImage(), pos + Frame::Vec2 { 32.f, -20.f }.RotateDegree(rot) * scale, baseColor, alpha, scale, rot + 45.f);
	} else if(type == IDeviceData::Joint) {
		__DrawDeviceSprite(joint_top_color, colorSet.color2);
		__DrawDeviceSprite(joint_top, baseColor);
	}

#undef __DrawDeviceSprite
#undef __DrawDeviceSprite_ByPart

}

void CEditorComponent::DrawOperationPrompt(const Frame::Vec2 & leftBottom) {
	if(!m_pFont) {
		return;
	}

	Texts::EText additionalText = Texts::EText::EMPTY;

	switch(m_tool) {
	case ETool::Hand: additionalText = Texts::EText::EditorOperationPrompt_Hand; break;
	case ETool::Pencil: additionalText = Texts::EText::EditorOperationPrompt_Pencil; break;
	case ETool::Pipe:
		additionalText = Texts::EText::EditorOperationPrompt_Pipe;
		if(m_pipeToolMode == EPipeToolMode::Pencil && !m_pipeNodesEditing.empty()) {
			additionalText = Texts::EText::EditorOperationPrompt_PipePencil_Drawing;
		}
		break;
	case ETool::Controller:
		if(m_toolControllerStuff.pEDCompWaitingForKey) {
			additionalText = Texts::EText::EditorOperationPrompt_Controller_Setting;
		}
		break;
	}

	const UnicodeString & cameraStr = Texts::GetText(Texts::EText::EditorOperationPrompt_Camera);
	Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(cameraStr, leftBottom, Frame::ETextHAlign::Left, Frame::ETextVAlign::Bottom, 0x000000, 1.f);
	if(additionalText != Texts::EText::EMPTY) {
		const UnicodeString & additionalStr = Texts::GetText(additionalText);
		//const float additionalTextHeight = m_pFont->TextHeight(additionalStr, 0.f);
		const float cameraTextHeight = m_pFont->TextHeight(cameraStr, 0.f) + 4.f;
		Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(additionalStr, leftBottom - Frame::Vec2 { 0.f, cameraTextHeight }, Frame::ETextHAlign::Left, Frame::ETextVAlign::Bottom, 0x000000, 1.f);
	}
}

void CEditorComponent::ButtonEnd(const Frame::Vec2 & rightBottom) {
	const UnicodeString & str = Texts::GetText(Texts::EText::EditorEnd);
	const Frame::Vec2 sizeHalf = m_pFont->TextSize(str, 0.f) * .5f + Frame::Vec2 { 16.f, 8.f };
	const Frame::Vec2 pos = rightBottom - sizeHalf - 8.f;
	const bool bMouseOn = Frame::PointInRectangle(GetMousePosInScene(), pos - sizeHalf, pos + sizeHalf);
	__DRAW_TEXT_BUTTON(pos, sizeHalf, bMouseOn, str);
	if(bMouseOn) {
		m_bMouseOnGUI = true;
		if(m_bMBLeftPressed) {
			SetWorking(false);
			SummonMachine();
		}
	}
}

void CEditorComponent::SummonMachine() {
	auto it = std::find_if(m_editorDeviceComponents.begin(), m_editorDeviceComponents.end(),
		[](CEditorDeviceComponent * pEDComp) {
			return pEDComp && pEDComp->GetDeviceType() == IDeviceData::EType::Cabin;
		}
	);
	if(it == m_editorDeviceComponents.end()) {
		return;
	}
	if(auto pEnt = Frame::gEntitySystem->SpawnEntity()) {
		if(auto pComp = pEnt->CreateComponent<CMachineComponent>()) {
			pComp->Initialize(* it, m_pipes, GetCurrentColorSet());
		}
	}
}

#undef __DRAW_TEXT_BUTTON