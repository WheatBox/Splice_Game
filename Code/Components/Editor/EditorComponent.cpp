﻿#include "EditorComponent.h"

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

REGISTER_ENTITY_COMPONENT(CEditorComponent);

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

CEditorComponent * CEditorComponent::s_pEditorComponent = nullptr;

void CEditorComponent::Initialize() {
	m_pEntity->SetZDepth(Depths::Editor);

	SetWorking(true);

	Put({ 0.f }, IDeviceData::EType::Cabin, 0);

	m_pFont = new Frame::CFont { Assets::GetFontFilename(), 16.f };

	m_pCameraComponent = m_pEntity->CreateComponent<CCameraComponent>();
	if(m_pCameraComponent) {
		m_pCameraComponent->Initialize(
			[this]() {
				return (m_bMBLeftHolding && m_tool == ETool::Hand) || Frame::gInput->pMouse->GetHolding(Frame::EMouseButtonId::eMBI_Middle);
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

	InitGUI();

	s_pEditorComponent = this;
}

void CEditorComponent::OnShutDown() {
	// TODO - 清理所有 SEditorPipeNode
	// 和 CEditorDeviceComponent
	// 和 CEditorDeviceComponent 的 CEntity

	if(s_pEditorComponent == this) {
		s_pEditorComponent = nullptr;
	}

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
		m_bMouseOnGUI = GUI::gGUIMouseData.bMouseOnGUI;
		if(!m_bMouseOnGUI) {
			m_bMBLeftPressed = GUI::gGUIMouseData.bMBLeftPressed;
			m_bMBLeftHolding = GUI::gGUIMouseData.bMBLeftHolding;
			m_bMBLeftReleased = GUI::gGUIMouseData.bMBLeftReleased;
			m_bMBRightPressed = Frame::gInput->pMouse->GetPressed(Frame::eMBI_Right);
		} else {
			m_bMBLeftPressed = false;
			m_bMBLeftHolding = false;
			m_bMBLeftReleased = false;
			m_bMBRightPressed = false;
		}

		if(m_pCameraComponent) {
			m_pCameraComponent->CameraControl(!m_bMouseOnGUI);
		}

		DrawBlockBackground();
	}
	break;
	case Frame::EntityEvent::Render:
	{
		Frame::gRenderer->pTextRenderer->SetFont(m_pFont);

		{
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

		m_pToolPencilMenu->SetShowing(false);
		m_pToolPipeMenu->SetShowing(false);
		m_pToolSwatchesMenu->SetShowing(false);
		m_pToolSwatchesColorEditor->SetShowing(false);
		m_pToolControllerMenu->SetShowing(false);
		//m_pToolControllerController->SetShowing(false);
		switch(m_tool) {
		case ETool::Pencil:
			m_pToolPencilMenu->SetShowing(true);
			break;
		case ETool::Pipe:
			m_pToolPipeMenu->SetShowing(true);
			break;
		case ETool::Swatches:
			m_pToolSwatchesMenu->SetShowing(true);
			m_pToolSwatchesColorEditor->SetShowing(true);
			break;
		case ETool::Controller:
			m_pToolControllerMenu->SetShowing(true);
			//m_pToolControllerController->SetShowing(true);
			break;
		}

		// ---- Legacy ----

		const float camZoomBeforeGUI = Frame::gCamera->GetZoom();
		const Frame::Vec2 camPosBeforeGUI = Frame::gCamera->GetPos();
		const float camRotBeforeGUI = Frame::gCamera->GetRotation();
		Frame::gCamera->SetZoom(1.f);
		const Frame::Vec2 viewSize = Frame::Vec2Cast(Frame::gCamera->GetViewSize());
		Frame::gCamera->SetPos(viewSize * .5f);
		Frame::gCamera->SetRotation(0.f);

		/*if(m_bMouseOnGUI) {
			m_bMBLeftPressed = bMBLeftPressed;
			m_bMBLeftHolding = bMBLeftHolding;
			m_bMBLeftReleased = bMBLeftReleased;
			m_bMBRightPressed = bMBRightPressed;
		}*/

		//m_bMouseOnGUI = false;

		const Frame::Vec2 toolbarLeftTopPos = { 0.f };//Frame::gCamera->GetPos() - viewSize * .5f;
		const Frame::Vec2 toolbarRightBottomPos = toolbarLeftTopPos + Frame::Vec2 { toolbarWidth, viewSize.y };

		Frame::Vec2 toolMenuLT = { toolbarRightBottomPos.x, toolbarLeftTopPos.y };
		switch(m_tool) {
		case ETool::Controller:
			RenderAndProcessControllerMenu(toolMenuLT);
			ButtonEnd(toolbarLeftTopPos + viewSize);
			break;
		}

		if(m_mouseLabelText != Texts::EText::EMPTY) {
			Texts::DrawTextLabel(m_mouseLabelText, Frame::gInput->pMouse->GetPosition() + Frame::Vec2 { 16.f, 0.f });
			m_mouseLabelText = Texts::EText::EMPTY;
		}

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
	
	GUI::SetOrUnsetGUI(m_pGUI, b);
}

void CEditorComponent::InitGUI() {
	m_pGUI = std::make_shared<GUI::CGUI>();

	InitGUI_Toolbar();
	InitGUI_ToolPencilMenu();
	InitGUI_ToolPipeMenu();
	InitGUI_ToolSwatchesMenu();
	InitGUI_ToolSwatchesColorEditor();
	InitGUI_ToolControllerMenu();
	InitGUI_ToolControllerController();
	InitGUI_OperationPrompt();

	GUI::SetGUI(m_pGUI);
}

void CEditorComponent::InitGUI_Toolbar() {
	const ETool arrTools[] = { ETool::Hand, ETool::Pencil, ETool::Eraser, ETool::Pipe, ETool::Swatches, ETool::Controller };
	const Assets::EGUIStaticSprite arrSprs[] = {
		Assets::EGUIStaticSprite::Editor_tool_hand,
		Assets::EGUIStaticSprite::Editor_tool_pencil,
		Assets::EGUIStaticSprite::Editor_tool_eraser,
		Assets::EGUIStaticSprite::Editor_tool_pipe,
		Assets::EGUIStaticSprite::Editor_tool_swatches,
		Assets::EGUIStaticSprite::Editor_tool_controller
	};
	const Texts::EText arrTexts[] = {
		Texts::EText::EditorToolHand,
		Texts::EText::EditorToolPencil,
		Texts::EText::EditorToolEraser,
		Texts::EText::EditorToolPipe,
		Texts::EText::EditorToolSwatches,
		Texts::EText::EditorToolController
	};

	constexpr float pageWidth = 48.f;
	constexpr float pageHeight = 320.f;
	constexpr float buttonSize = pageWidth - 8.f;

	auto p = m_pGUI->CreateElement<GUI::CDraggablePage>(12.f, Frame::Vec2 { pageWidth, pageHeight });
	float y = 48.f;
	int i = 0;
	for(ETool tool : arrTools) {
		auto pCurrButton = p->CreateElement<GUI::CImageButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[tool]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SwitchTool(tool); } },
			arrSprs[i],
			arrTexts[i]
		);
		pCurrButton->SetCustomHighlightingCondition([tool]() {
			if(CEditorComponent::s_pEditorComponent) {
				return CEditorComponent::s_pEditorComponent->m_tool == tool;
			}
			return false;
			});
		y += (buttonSize + pageWidth) * .5f;
		i++;
	}

	p->SetShowing(true);
}

void CEditorComponent::InitGUI_ToolPencilMenu() {
	constexpr float pageWidth = 96.f;
	constexpr float pageHeight = 512.f;
	constexpr float buttonSize = pageWidth - 16.f;

	auto & p = m_pToolPencilMenu;
	p = m_pGUI->CreateElement<GUI::CDraggablePage>(Frame::Vec2 { 72.f, 12.f }, Frame::Vec2 { pageWidth, pageHeight });

	float y = buttonSize * .5f + 8.f;
	for(IDeviceData::EType type = IDeviceData::Shell; type < IDeviceData::END; type = static_cast<IDeviceData::EType>(type + 1)) { // 只显示从 Shell 开始的装置
		float spriteScale = 1.f;
		switch(type) {
		case IDeviceData::Shell:
		case IDeviceData::Engine: spriteScale = .8f; break;
		case IDeviceData::Propeller: spriteScale = .4f; break;
		case IDeviceData::JetPropeller: spriteScale = .5f; break;
		case IDeviceData::Joint: spriteScale = .8f; break;
		}
		spriteScale *= .7f;

		auto pCurrButton = p->CreateElement<GUI::CCustomDrawingButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[type]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SwitchPencilDevice(type); } },
			[type, spriteScale](const Frame::Vec2 & center) {
				if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->DrawDevicePreview(type, center, 1.f, 0, spriteScale); }
			}
		);
		pCurrButton->SetCustomHighlightingCondition(
			[type]() {
				if(CEditorComponent::s_pEditorComponent) {
					return CEditorComponent::s_pEditorComponent->m_pencilDevice == type;
				}
				return false;
			}
		);
		
		y += buttonSize + 6.f;
	}

	p->SetShowing(false);
}

void CEditorComponent::InitGUI_ToolPipeMenu() {
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

	constexpr float pageWidth = 48.f;
	constexpr float pageHeight = 168.f;
	constexpr float buttonSize = pageWidth - 8.f;
	auto & p = m_pToolPipeMenu;
	p = m_pGUI->CreateElement<GUI::CDraggablePage>(Frame::Vec2 { 72.f, 12.f }, Frame::Vec2 { pageWidth, pageHeight });
	float y = 48.f;
	int i = 0;
	for(EPipeToolMode mode : arrModes) {
		auto pCurrButton = p->CreateElement<GUI::CImageButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[mode]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SwitchPipeToolMode(mode); } },
			arrSprs[i],
			arrTexts[i]
		);
		pCurrButton->SetCustomHighlightingCondition([mode]() {
			if(CEditorComponent::s_pEditorComponent) {
				return CEditorComponent::s_pEditorComponent->m_pipeToolMode == mode;
			}
			return false;
			});
		y += (buttonSize + pageWidth) * .5f;
		i++;
	}

	p->SetShowing(false);
}

void CEditorComponent::InitGUI_ToolSwatchesMenu() {
	constexpr float pageWidth = 160.f;
	constexpr float pageHeight = 512.f;
	const Frame::Vec2 buttonSize { pageWidth - 16.f, 44.f };

	auto & p = m_pToolSwatchesMenu;
	p = m_pGUI->CreateElement<GUI::CDraggablePage>(Frame::Vec2 { 72.f, 12.f }, Frame::Vec2 { pageWidth, pageHeight });
	float y = buttonSize.y * .5f + 8.f;
	for(size_t i = 0, len = m_colorSets.size(); i < len; i++) {
		auto pCurrButton = p->CreateElement<GUI::CCustomDrawingButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[i]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SetCurrentColorSetByIndex(i); } },
			[i](const Frame::Vec2 & center) {
				if(!CEditorComponent::s_pEditorComponent) {
					return;
				}
				if(i >= CEditorComponent::s_pEditorComponent->m_colorSets.size()) {
					return;
				}

				const auto & colorSet = CEditorComponent::s_pEditorComponent->m_colorSets[i];

#define __DrawColorBlock(memberVarInColorSet, offsetMultiply) \
{ \
	Frame::Vec2 colorBlockCenter = center + Frame::Vec2 { offsetMultiply * 34.f, 0.f }; \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorBlockCenter - 14.f, colorBlockCenter + 14.f, colorSet.memberVarInColorSet, 1.f); \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(colorBlockCenter - 14.f, colorBlockCenter + 14.f, 0xFFFFFF, 1.f, 1.f); \
}
				__DrawColorBlock(color1, -1.5f);
				__DrawColorBlock(color2, -.5f);
				__DrawColorBlock(connector, .5f);
				__DrawColorBlock(pipe, 1.5f);

#undef __DrawColorBlock
			}
		);
		pCurrButton->SetCustomHighlightingCondition([i]() {
			if(CEditorComponent::s_pEditorComponent) {
				return CEditorComponent::s_pEditorComponent->m_currColorSetIndex == i;
			}
			return false;
			});
		pCurrButton->SetHighlightStyle(GUI::CButtonBase::EHighlightStyle::Outline);
		y += buttonSize.y + 6.f;
	}

	p->SetShowing(false);
}

static Frame::ColorRGB & __Color(Frame::ColorRGB SColorSet::* memberVarInColorSet = nullptr) {
	static Frame::ColorRGB defaultRes;
	if(!CEditorComponent::s_pEditorComponent) {
		return defaultRes;
	}
	if(!memberVarInColorSet) {
		memberVarInColorSet = CEditorComponent::s_pEditorComponent->GetColorSetEditingColor();
	}
	return CEditorComponent::s_pEditorComponent->GetColorSetEditing().* memberVarInColorSet;
}

static uint8 & __ColorPart(uint8 Frame::ColorRGB::* colorPart) {
	return __Color().* colorPart;
}

static void __CreateColorButton(const std::shared_ptr<GUI::CDraggablePage> & p, float yBase, float offsetMultiply, Frame::ColorRGB SColorSet::* memberVarInColorSet) {
	auto pButton = p->CreateElement<GUI::CCustomDrawingButton>(Frame::Vec2 { 64.f + 48.f * offsetMultiply, yBase }, 32.f,
		[memberVarInColorSet]() {
			if(!CEditorComponent::s_pEditorComponent) return;
			CEditorComponent::s_pEditorComponent->SetColorSetEditingColor(memberVarInColorSet);
		},
		[memberVarInColorSet](const Frame::Vec2 & buttonCenter) {
			if(!CEditorComponent::s_pEditorComponent) return;
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(buttonCenter - 16.f, buttonCenter + 16.f, __Color(memberVarInColorSet), 1.f);
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(buttonCenter - 16.f, buttonCenter + 16.f, 0xFFFFFF, 1.f, 1.f);
		});
	pButton->SetCustomHighlightingCondition([memberVarInColorSet]() {
		if(!CEditorComponent::s_pEditorComponent) return false;
		return CEditorComponent::s_pEditorComponent->GetColorSetEditingColor() == memberVarInColorSet;
		});
	pButton->SetHighlightStyle(GUI::CCustomDrawingButton::EHighlightStyle::Outline);
	pButton->SetOutlineHighlightOutlineWidth(3.f);
}

static void __CreateColorBarAndOtherStuffs(const std::shared_ptr<GUI::CDraggablePage> & p, float yBase, float offsetMultiply, uint8 Frame::ColorRGB::* colorPart) {
	const float _y = yBase + 44.f + offsetMultiply * 32.f;
	auto pSlider = p->CreateElement<GUI::CSlider>(Frame::Vec2 { 148.f, _y }, 256.f, 255,
		GUI::CSlider::FuncSynchValue {},
		GUI::CSlider::FuncOnValueChanged {}
	).get();

	bool isR = colorPart == & Frame::ColorRGB::r;
	bool isG = colorPart == & Frame::ColorRGB::g;
	bool isB = colorPart == & Frame::ColorRGB::b;

	pSlider->SetFuncOnValueChanged([pSlider, colorPart, isR, isG, isB](int value, int) {
		uint8 val = static_cast<uint8>(value);
		__ColorPart(colorPart) = val;
		pSlider->SetBackgroundColor(Frame::ColorUniteRGB(val * isR, val * isG, val * isB));
		});
	pSlider->SetFuncSynchValue([pSlider, colorPart, isR, isG, isB]() -> uint8 {
		uint8 val = static_cast<uint8>(pSlider->GetValue());
		pSlider->SetBackgroundColor(Frame::ColorUniteRGB(val * isR, val * isG, val * isB));
		return __ColorPart(colorPart);
		});

	auto pButtonDec = p->CreateElement<GUI::CTextButton>(Frame::Vec2 { 304.f, _y }, 28.f, [colorPart]() {
		__ColorPart(colorPart)--;
		}, Texts::EText::EMPTY);
	pButtonDec->SetText("[-]");
	auto pButtonInc = p->CreateElement<GUI::CTextButton>(Frame::Vec2 { 336.f, _y }, 28.f, [colorPart]() {
		__ColorPart(colorPart)++;
		}, Texts::EText::EMPTY);
	pButtonInc->SetText("[+]");

	auto pLabel = p->CreateElement<GUI::CLabel>(Frame::Vec2 { 360.f, _y }, Texts::EText::EMPTY, Frame::ETextHAlign::Left, Frame::ETextVAlign::Middle);
	pLabel->SetFuncStep([colorPart](GUI::CLabel * me) {
		if(!CEditorComponent::s_pEditorComponent) return;
		me->SetText(std::to_string(__ColorPart(colorPart)));
		});
}

void CEditorComponent::InitGUI_ToolSwatchesColorEditor() {
	constexpr float pageWidth = 400.f;
	constexpr float pageHeight = 232.f;

	auto & p = m_pToolSwatchesColorEditor;
	p = m_pGUI->CreateElement<GUI::CDraggablePage>(Frame::Vec2 { 244.f, 12.f }, Frame::Vec2 { pageWidth, pageHeight });

	constexpr float yBase = 44.f;

	__CreateColorButton(p, yBase, 0.f, & SColorSet::color1);
	__CreateColorButton(p, yBase, 1.f, & SColorSet::color2);
	__CreateColorButton(p, yBase, 2.f, & SColorSet::connector);
	__CreateColorButton(p, yBase, 3.f, & SColorSet::pipe);

	__CreateColorBarAndOtherStuffs(p, yBase, 0.f, & Frame::ColorRGB::r);
	__CreateColorBarAndOtherStuffs(p, yBase, 1.f, & Frame::ColorRGB::g);
	__CreateColorBarAndOtherStuffs(p, yBase, 2.f, & Frame::ColorRGB::b);

	constexpr float btApplyY = yBase + 160.f;
	p->CreateElement<GUI::CLabel>(Frame::Vec2 { 64.f, btApplyY - 12.f }, Texts::EText::EMPTY, Frame::ETextHAlign::Left, Frame::ETextVAlign::Middle)->SetFuncStep(
		[](GUI::CLabel * me) {
			const auto & colorEditing = __Color();
			char szColorCode[8];
#ifdef _WIN32
			sprintf_s(szColorCode, 8, "#%02X%02X%02X", colorEditing.r, colorEditing.g, colorEditing.b);
#else
			sprintf(szColorCode, "#%02X%02X%02X", colorEditing.r, colorEditing.g, colorEditing.b);
#endif
			me->SetText(szColorCode);
		});
	p->CreateElement<GUI::CTextButton>(Frame::Vec2 { 304.f, btApplyY }, Frame::Vec2 { 64.f, 24.f }, []() {
		if(!CEditorComponent::s_pEditorComponent) return;
		CEditorComponent::s_pEditorComponent->SetCurrentColorSetByIndex(CEditorComponent::s_pEditorComponent->m_currColorSetIndex);
		}, Texts::EText::Reset);
	p->CreateElement<GUI::CTextButton>(Frame::Vec2 { 224.f, btApplyY }, Frame::Vec2 { 64.f, 24.f }, []() {
		if(!CEditorComponent::s_pEditorComponent) return;
		CEditorComponent::s_pEditorComponent->ApplyCurrentColorSet();
		}, Texts::EText::Apply);
	
	p->SetShowing(false);
}

void CEditorComponent::InitGUI_ToolControllerMenu() {
	constexpr float pageWidth = 96.f;
	constexpr float pageHeight = 512.f;
	constexpr float buttonSize = pageWidth - 16.f;

	auto & p = m_pToolControllerMenu;
	p = m_pGUI->CreateElement<GUI::CDraggablePage>(Frame::Vec2 { 72.f, 12.f }, Frame::Vec2 { pageWidth, pageHeight });

	float y = buttonSize * .5f + 8.f;
	int i = 0;
	for(Controller::EElement elem = Controller::EElement::Button; elem < Controller::EElement::END; elem = static_cast<Controller::EElement>(static_cast<int>(elem) + 1)) {
		float spriteScale = 1.f;
		switch(elem) {
		case Controller::EElement::Button: spriteScale = 1.f; break;
		}

		auto pCurrButton = p->CreateElement<GUI::CCustomDrawingButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[elem]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SwitchControllerPencilElement(elem); } },
			[elem, spriteScale](const Frame::Vec2 & center) {
				Controller::DrawPreview(elem, center, 1.f, spriteScale);
			}
		);
		pCurrButton->SetCustomHighlightingCondition(
			[elem]() {
				if(CEditorComponent::s_pEditorComponent) {
					return CEditorComponent::s_pEditorComponent->m_controllerPencilElement == elem;
				}
				return false;
			}
		);

		y += buttonSize + 6.f;
		i++;
	}

	p->SetShowing(false);
}

void CEditorComponent::InitGUI_ToolControllerController() {
	//auto & p = m_pToolControllerController;
	//p = m_pGUI->CreateElement<GUI::CDraggableResizablePage>(Frame::Vec2 { 244.f, 12.f }, Controller::gridCellSize, m_controllerEditing.gridSize, Frame::Vec2i { Controller::controllerMinWidth, Controller::controllerMinHeight });
	//TODO

	//const Frame::Vec2 mousePosInScene = GetMousePosInScene();
	//const Frame::Vec2 controllerLT = p->GetL

	//p->SetShowing(false);
}

void CEditorComponent::InitGUI_OperationPrompt() {
	auto pFixed = m_pGUI->CreateElement<GUI::CLabel>(0.f, Texts::EText::EditorOperationPrompt_Camera, Frame::ETextHAlign::Left, Frame::ETextVAlign::Bottom);
	pFixed->SetFuncStep([](GUI::CLabel * me) {
		me->SetPos({ 20.f, static_cast<float>(Frame::gCamera->GetWindowSize().y) - 16.f });
		});
	pFixed->SetColor(0x000000);

	auto pDynamic = m_pGUI->CreateElement<GUI::CLabel>(0.f, Texts::EText::EMPTY, Frame::ETextHAlign::Left, Frame::ETextVAlign::Bottom);
	pDynamic->SetFuncStep([](GUI::CLabel * me) {
		me->SetPos({ 20.f, static_cast<float>(Frame::gCamera->GetWindowSize().y) - 36.f });

		if(!CEditorComponent::s_pEditorComponent) {
			return;
		}

		Texts::EText text = Texts::EText::EMPTY;

		switch(CEditorComponent::s_pEditorComponent->m_tool) {
		case ETool::Hand: text = Texts::EText::EditorOperationPrompt_Hand; break;
		case ETool::Pencil: text = Texts::EText::EditorOperationPrompt_Pencil; break;
		case ETool::Pipe:
			text = Texts::EText::EditorOperationPrompt_Pipe;
			if(CEditorComponent::s_pEditorComponent->m_pipeToolMode == EPipeToolMode::Pencil && !CEditorComponent::s_pEditorComponent->m_pipeNodesEditing.empty()) {
				text = Texts::EText::EditorOperationPrompt_PipePencil_Drawing;
			}
			break;
		case ETool::Controller:
			if(CEditorComponent::s_pEditorComponent->m_toolControllerStuff.pEDCompWaitingForKey) {
				text = Texts::EText::EditorOperationPrompt_Controller_Setting;
			}
			break;
		}

		me->SetText(text);
		});
	pDynamic->SetColor(0x000000);
}

void CEditorComponent::RenderAndProcessControllerMenu(const Frame::Vec2 & leftTopPos) {
	constexpr float menuWidth = GetToolMenuWidth(ETool::Controller);

	const Frame::Vec2 mousePosInScene = GetMousePosInScene();

	/* ----------------------- 底盘 ----------------------- */

	const Frame::Vec2 controllerSize = Vec2Cast(Frame::Vec2i { m_controllerEditing.gridSize.x, m_controllerEditing.gridSize.y } * Controller::gridCellSize);

	const Frame::Vec2 controllerLT = m_controllerMenuDragger.GetLeftTop();
	const Frame::Vec2 controllerRB = controllerLT + controllerSize;
	
	m_controllerMenuDragger.WorkPart1(mousePosInScene, controllerSize, { leftTopPos.x + menuWidth, leftTopPos.y }, Frame::Vec2Cast<float>(Frame::gCamera->GetViewSize()));
	if(m_controllerMenuDragger.IsDragging()) {
		m_bMBRightPressed = m_bMBLeftPressed = m_bMBLeftHolding = false;
	}

	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(controllerLT, controllerRB, 0x4F4F4F, 1.f);
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(controllerLT, controllerRB, __GUI_BACKGROUND_EDGE_COLOR, 1.f, 1.f);

	Frame::gRenderer->pShapeRenderer->DrawLineBlended({ controllerRB.x, controllerRB.y - 12.f }, { controllerRB.x - 12.f, controllerRB.y }, 0xFFFFFF, 1.f, 2.f);
	Frame::gRenderer->pShapeRenderer->DrawLineBlended({ controllerRB.x, controllerRB.y - 6.f }, { controllerRB.x - 6.f, controllerRB.y }, 0xFFFFFF, 1.f, 2.f);

	/* ----------------------- 拖动角落调整底盘尺寸 ----------------------- */

	if(Frame::PointInRectangle(mousePosInScene, controllerRB - 8.f, controllerRB + 8.f)) {
		gApplication->SetCursor(CApplication::eCursor_ResizeNWSE);
		if(m_bMBLeftPressed) {
			m_bMBLeftPressed = m_bMBLeftHolding = false;
			m_bControllerResizing = true;
			m_controllerResizingMinSize = { Controller::controllerMinWidth, Controller::controllerMinHeight };
			for(auto & pElem : m_controllerEditing.elements) {
				const Frame::Vec2i elemRB = pElem->GetAABB().second;
				m_controllerResizingMinSize.x = std::max(elemRB.x, m_controllerResizingMinSize.x);
				m_controllerResizingMinSize.y = std::max(elemRB.y, m_controllerResizingMinSize.y);
			}
		}
	}

	if(m_bControllerResizing) {
		gApplication->SetCursor(CApplication::eCursor_ResizeNWSE);

		Frame::Vec2i newSize = Frame::Vec2Cast<int>((mousePosInScene - controllerLT) / static_cast<float>(Controller::gridCellSize) + .5f);
		m_controllerEditing.gridSize.x = std::max(newSize.x, m_controllerResizingMinSize.x);
		m_controllerEditing.gridSize.y = std::max(newSize.y, m_controllerResizingMinSize.y);
		
		if(m_bMBLeftReleased) {
			m_bControllerResizing = false;
		}
	}

	/* ----------------------- 拖动或右键现有元件 ----------------------- */
	
	if(!m_pDraggingControllerElement && !m_bControllerResizing) {
		for(auto & pElem : m_controllerEditing.elements) {
			if(auto [lt, rb] = Controller::GetElementAABBRealPos(pElem, controllerLT); !Frame::PointInRectangle(mousePosInScene, lt, rb)) {
				continue;
			}
			if(m_bMBLeftPressed) {
				m_bMBLeftPressed = m_bMBLeftHolding = false;
				m_pDraggingControllerElement = pElem;
				m_draggingControllerElementPosRelativeToMouse = Controller::GetElementRealPos(pElem, controllerLT) - mousePosInScene;
				break;
			} else if(m_bMBRightPressed) {
				m_bMBRightPressed = false;
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
				m_draggingControllerElementPosRelativeToMouse = 0.f;
			}
		}
	}

	/* ----------------------- 拖动并放置元件 ----------------------- */

	if(m_pDraggingControllerElement) {
		Frame::Vec2i posDraggingTo;
		{
			Frame::Vec2i temp = Frame::Vec2Cast<int>(mousePosInScene + m_draggingControllerElementPosRelativeToMouse - controllerLT) + Controller::gridCellSize / 2;
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
		if(Controller::AABBIntersect(aabbLT, aabbRB, 0, { m_controllerEditing.gridSize.x, m_controllerEditing.gridSize.y })) {
			bCanPut = true;
			if(aabbLT.x < 0 || aabbLT.y < 0 || aabbRB.x > m_controllerEditing.gridSize.x || aabbRB.y > m_controllerEditing.gridSize.y) {
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
			const Frame::Vec2 previewPos = mousePosInScene + m_draggingControllerElementPosRelativeToMouse;

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
		//m_bMouseOnGUI = true;
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
		m_colorSetEditing = GetCurrentColorSet();
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
		DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::propeller_blade_color)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.GetRotatedDegree(rot) * scale, colorSet.color2, alpha, rot + 30.f, dirIndex % 2 ? Frame::Vec2 { scale, .3f * scale } : Frame::Vec2 { .3f * scale, scale }, 0.f);
		DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::propeller_blade)->GetImage(), pos + Frame::Vec2 { 20.f, 0.f }.GetRotatedDegree(rot) * scale, baseColor, alpha, rot + 30.f, dirIndex % 2 ? Frame::Vec2 { scale, .3f * scale } : Frame::Vec2 { .3f * scale, scale }, 0.f);
		__DrawDeviceSprite(propeller_top_color, colorSet.color1);
		__DrawDeviceSprite(propeller_top, baseColor);
	} else if(type == IDeviceData::JetPropeller) {
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::jet_propeller_needle)->GetImage(), pos + Frame::Vec2 { 32.f, -20.f }.GetRotatedDegree(rot) * scale, baseColor, alpha, scale, rot + 45.f);
	} else if(type == IDeviceData::Joint) {
		__DrawDeviceSprite(joint_top_color, colorSet.color2);
		__DrawDeviceSprite(joint_top, baseColor);
	}

#undef __DrawDeviceSprite
#undef __DrawDeviceSprite_ByPart

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
