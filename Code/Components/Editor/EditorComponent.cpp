#include "EditorComponent.h"

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>
#include <FrameInput/Input.h>
#include <FrameRender/Renderer.h>
#include <FrameCore/Camera.h>
#include <FrameMath/Math.h>

#include "../CameraComponent.h"
#include "../Machine/DeviceConnectorRendererComponent.h"
#include "../Machine/MachinePartComponent.h"
#include "../Machine/MachineComponent.h"
#include "../PhysicsWorldComponent.h"

#include "../../Depths.h"
#include "../../Pipe.h"

#include <algorithm>

#include <nlohmann/json.hpp>

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

constexpr float __INTERFACE_CONNECT_DISTANCE = CONNECTOR_LENGTH + 4.f;

#define __DRAW_TEXT_BUTTON(center, sizehalf, highlight, text) { \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(center - sizehalf, center + sizehalf, highlight ? __GUI_BUTTON_HIGHLIGHT_COLOR : __GUI_BUTTON_COLOR, 1.f); \
	Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(center - sizehalf, center + sizehalf, __GUI_BUTTON_EDGE_COLOR, 1.f, 1.f); \
	Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(text, center + Frame::Vec2 { .0f, -1.f }, Frame::ETextHAlign::Center, Frame::ETextVAlign::Middle, __GUI_BUTTON_TEXT_COLOR, 1.f); \
}

CEditorComponent * CEditorComponent::s_pEditorComponent = nullptr;

void CEditorComponent::Initialize() {
	m_pencilDeviceIndex = GetEditorDeviceConfig<SShellEditorDeviceData>().orderIndex;

	m_pEntity->SetZDepth(Depths::Editor);

	SetWorking(true);

	//__Put(m_editorDeviceComponents, GetCurrentColorSet(), 0.f, 0, 0, true);
	//DeserializeEditorMachine("[{\"connections\":[],\"guidhigh\":11124632734042574262,\"guidlow\":11640985875138584614,\"rot\":0.0,\"x\":0.0,\"y\":0.0}]");
	DeserializeEditorMachine("[{\"connections\":[],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-4.71238899230957,\"x\":-111.99999237060547,\"y\":112.0},{\"connections\":[],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-7.853981971740723,\"x\":-112.00003051757813,\"y\":-112.0},{\"connections\":[],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-1.5707963705062866,\"x\":-7.693204679526389e-06,\"y\":-224.0},{\"connections\":[{\"ID\":1,\"to\":0,\"toID\":2},{\"ID\":3,\"to\":1,\"toID\":2}],\"guidhigh\":13541370941465250790,\"guidlow\":11506182755973392982,\"rot\":-3.1415927410125732,\"x\":-112.0,\"y\":5.595057700702455e-06},{\"connections\":[{\"ID\":2,\"to\":3,\"toID\":2}],\"guidhigh\":11124632734042574262,\"guidlow\":11640985875138584614,\"rot\":0.0,\"x\":0.0,\"y\":0.0},{\"connections\":[{\"ID\":2,\"to\":4,\"toID\":0}],\"guidhigh\":13541370941465250790,\"guidlow\":11506182755973392982,\"rot\":-0.0,\"x\":112.0,\"y\":0.0},{\"connections\":[],\"guidhigh\":4725241458380522226,\"guidlow\":13457049288530237759,\"rot\":-4.71238899230957,\"x\":2.098779077641666e-06,\"y\":224.0},{\"connections\":[{\"ID\":1,\"to\":1,\"toID\":3},{\"ID\":0,\"to\":2,\"toID\":2},{\"ID\":2,\"to\":4,\"toID\":1}],\"guidhigh\":13541370941465250790,\"guidlow\":11506182755973392982,\"rot\":-1.5707963705062866,\"x\":-2.7975288503512274e-06,\"y\":-112.0},{\"connections\":[{\"ID\":2,\"to\":2,\"toID\":0}],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-1.5707963705062866,\"x\":-1.2588880053954199e-05,\"y\":-336.0},{\"connections\":[{\"ID\":2,\"to\":5,\"toID\":3}],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-4.71238899230957,\"x\":112.0,\"y\":112.0},{\"connections\":[{\"ID\":3,\"to\":0,\"toID\":1},{\"ID\":2,\"to\":4,\"toID\":3},{\"ID\":0,\"to\":6,\"toID\":0},{\"ID\":1,\"to\":9,\"toID\":3}],\"guidhigh\":13541370941465250790,\"guidlow\":11506182755973392982,\"rot\":-4.71238899230957,\"x\":7.631923608641955e-07,\"y\":112.0},{\"connections\":[{\"ID\":2,\"to\":5,\"toID\":1},{\"ID\":1,\"to\":7,\"toID\":3}],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-1.5707963705062866,\"x\":112.0,\"y\":-112.0},{\"connections\":[{\"ID\":1,\"to\":0,\"toID\":3}],\"guidhigh\":16370282976155157689,\"guidlow\":10090654385005652104,\"rot\":-4.71238899230957,\"x\":-224.0,\"y\":156.00001525878906},{\"connections\":[{\"ID\":2,\"to\":9,\"toID\":1}],\"guidhigh\":16370282976155157689,\"guidlow\":10090654385005652104,\"rot\":-4.71238899230957,\"x\":224.0,\"y\":156.0},{\"connections\":[{\"ID\":0,\"to\":1,\"toID\":1}],\"guidhigh\":4725241458380522226,\"guidlow\":13457049288530237759,\"rot\":-9.42477798461914,\"x\":-224.00003051757813,\"y\":-111.99998474121094},{\"connections\":[{\"ID\":0,\"to\":11,\"toID\":3}],\"guidhigh\":4725241458380522226,\"guidlow\":13457049288530237759,\"rot\":-6.2831854820251465,\"x\":224.0,\"y\":-112.00000762939453},{\"connections\":[{\"ID\":2,\"to\":1,\"toID\":0},{\"ID\":3,\"to\":2,\"toID\":1}],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-7.853981971740723,\"x\":-112.00006866455078,\"y\":-224.0},{\"connections\":[{\"ID\":2,\"to\":2,\"toID\":3},{\"ID\":3,\"to\":11,\"toID\":0}],\"guidhigh\":15774089909195984796,\"guidlow\":13193609732936951746,\"rot\":-6.2831854820251465,\"x\":111.99999237060547,\"y\":-224.00001525878906}]");

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

	for(auto & pEditorDeviceComp : m_editorDeviceComponents) {
		gApplication->RemoveEntityAtTheEndOfThisFrame(pEditorDeviceComp->GetEntity()->GetId());
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

		/* --------------------- HotKeys --------------------- */

		do {
			if(m_toolControllerStuff.pEDCompWaitingForKey) {
				break;
			}

			if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_1)) SwitchTool(ETool::Hand);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_2)) SwitchTool(ETool::Pencil);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_3)) SwitchTool(ETool::Eraser);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_4)) SwitchTool(ETool::Swatches);
			else if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_5)) SwitchTool(ETool::Controller);

			if(m_tool == ETool::Pencil) {
				const bool bGoPrev = Frame::gInput->pKeyboard->GetPressed(Frame::eKI_Q);
				const bool bGoNext = Frame::gInput->pKeyboard->GetPressed(Frame::eKI_E);
				if(bGoPrev) {
					PencilPrevDevice();
				} else if(bGoNext) {
					PencilNextDevice();
				}
			}

		} while(false);

		if(Frame::gInput->pKeyboard->GetPressed(Frame::eKI_T)) {
			printf(SerializeEditorMachine().c_str());
		}

		/* ----------------------- Canvas ----------------------- */
		
		__RegenerateInsBuffers(); // TODO - 优化

		Frame::gRenderer->DrawTexturesInstanced(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::cabin)->GetImage()->GetTextureId(), CSpriteComponent::GetTextureVertexBufferForInstances(), m_insBuffers);

		if(m_tool == ETool::Pencil) {
			Pencil();
		} else
		if(m_tool == ETool::Eraser) {
			Eraser();
		}

		/* ----------------------- GUI ----------------------- */

		m_pToolPencilMenu->SetShowing(false);
		m_pToolSwatchesMenu->SetShowing(false);
		m_pToolSwatchesColorEditor->SetShowing(false);
		m_pToolControllerMenu->SetShowing(false);
		//m_pToolControllerController->SetShowing(false);
		switch(m_tool) {
		case ETool::Pencil:
			m_pToolPencilMenu->SetShowing(true);
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

		Frame::gCamera->PushOntoStack();

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

		Frame::gCamera->PopFromStack();

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
	InitGUI_ToolSwatchesMenu();
	InitGUI_ToolSwatchesColorEditor();
	InitGUI_ToolControllerMenu();
	InitGUI_ToolControllerController();
	InitGUI_OperationPrompt();

	GUI::SetGUI(m_pGUI);
}

void CEditorComponent::InitGUI_Toolbar() {
	const ETool arrTools[] = { ETool::Hand, ETool::Pencil, ETool::Eraser, ETool::Swatches, ETool::Controller };
	const Assets::EGUIStaticSprite arrSprs[] = {
		Assets::EGUIStaticSprite::Editor_tool_hand,
		Assets::EGUIStaticSprite::Editor_tool_pencil,
		Assets::EGUIStaticSprite::Editor_tool_eraser,
		Assets::EGUIStaticSprite::Editor_tool_swatches,
		Assets::EGUIStaticSprite::Editor_tool_controller
	};
	const Texts::EText arrTexts[] = {
		Texts::EText::EditorToolHand,
		Texts::EText::EditorToolPencil,
		Texts::EText::EditorToolEraser,
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
	for(size_t i = 0, size = GetEditorDeviceRegistry().size(); i < size; i++) {
		if(!GetEditorDeviceConfig(GetEditorDeviceOrder()[i]).pencilEnable) {
			continue;
		}
		
		const float spriteScale = .7f;

		auto pCurrButton = p->CreateElement<GUI::CCustomDrawingButton>(
			Frame::Vec2 { pageWidth * .5f, y },
			buttonSize,
			[i]() { if(CEditorComponent::s_pEditorComponent) { CEditorComponent::s_pEditorComponent->SwitchPencilDevice(i); } },
			[i, spriteScale, buttonSize](const Frame::Vec2 & center) {
				if(CEditorComponent::s_pEditorComponent) {
					const Frame::Vec2 & drawSize = GetEditorDeviceRegistry()[GetEditorDeviceOrder()[i]]->GetConfig().size;
					const float scale = buttonSize / (drawSize.x > drawSize.y ? drawSize.x : drawSize.y);
					CEditorComponent::s_pEditorComponent->DrawDevicePreview(i, center, 1.f, 0, spriteScale * scale);
				}
			}
		);
		pCurrButton->SetCustomHighlightingCondition(
			[i]() {
				if(CEditorComponent::s_pEditorComponent) {
					return CEditorComponent::s_pEditorComponent->m_pencilDeviceIndex == i;
				}
				return false;
			}
		);
		
		y += buttonSize + 6.f;
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
	if(m_pencilDeviceIndex >= GetEditorDeviceRegistry().size()) {
		return;
	}

	bool bPutFinished = false;
	const Frame::Vec2 mousePos = GetMousePosInScene();
	const SInterfaceSet * pInterfaceSetMouseOn = nullptr;
	const CEditorDeviceComponent::SInterface * pInterfaceMouseOn = nullptr;
	for(auto & interfaceSet : m_interfaces) {
		const Frame::Vec2 lt = interfaceSet.pos - __INTERFACESET_BUTTON_SIZE_HALF;
		const Frame::Vec2 rb = interfaceSet.pos + __INTERFACESET_BUTTON_SIZE_HALF;
		if(!m_bMouseOnGUI && !pInterfaceMouseOn && Frame::PointInRectangle(mousePos, lt, rb)) {
			pInterfaceSetMouseOn = & interfaceSet;

			float minRad = 999999.f;
			for(auto & interface : interfaceSet.interfaces) {
				if(float rad = std::abs(Frame::Vec2 { 1.f, 0.f }.Rotate(-interface->direction).IncludedAngle(interfaceSet.pos - GetMousePosInScene())); rad < minRad) {
					minRad = rad;
					pInterfaceMouseOn = interface;
				}
			}

			if(pInterfaceMouseOn) {
				if(m_pInterfaceMouseOn != pInterfaceMouseOn) {
					m_pInterfaceMouseOn = pInterfaceMouseOn;
					RefreshInterfaceCanPut(* pInterfaceMouseOn);
				}

				//DrawDevicePreview(m_pencilDeviceIndex, GetWillPutPos(* pInterfaceMouseOn), __PENCIL_BUTTON_ALPHA, -pInterfaceMouseOn->direction, 1.f, 0xFF0000, !m_bInterfaceCanPut);
				DrawDevicePreview(m_pencilDeviceIndex, GetWillPutPos(* pInterfaceMouseOn), __PENCIL_BUTTON_ALPHA, -pInterfaceMouseOn->direction, 1.f, m_bInterfaceCanPut ? 0xFFFFFF : 0xFF0000);
			}
		}
		if(pInterfaceSetMouseOn != & interfaceSet) {
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_COLOR, __INTERFACESET_BUTTON_ALPHA);
			Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __PENCIL_BUTTON_EDGE_COLOR, __INTERFACESET_BUTTON_ALPHA, 1.f);
		}
	}

	for(auto & interfaceSet : m_interfaces) {
		for(auto & interface : interfaceSet.interfaces) {
			if(interface->from == nullptr) {
				continue;
			}
			//Frame::Vec2 entityPos = interface.pEditorDeviceComponent->GetEntity()->GetPosition();
		
			Frame::Vec2 p1 { __INTERFACE_SIGN_WIDTH_HALF, __INTERFACE_SIGN_HEIGHT_HALF };
			Frame::Vec2 p2 { __INTERFACE_SIGN_WIDTH_HALF, -__INTERFACE_SIGN_HEIGHT_HALF };
			Frame::Vec2 p3 { -__INTERFACE_SIGN_WIDTH_HALF, 0.f };
			Frame::Rotate2DVectors(-interface->direction, { & p1, & p2, & p3 });
			p1 += interface->pos;
			p2 += interface->pos;
			p3 += interface->pos;
			Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(p1, p2, p3, __PENCIL_BUTTON_COLOR, __PENCIL_BUTTON_ALPHA);
			Frame::gRenderer->pShapeRenderer->DrawTriangleBlended(p1, p2, p3, __PENCIL_BUTTON_EDGE_COLOR, __PENCIL_BUTTON_ALPHA, 1.f);
			//Frame::gRenderer->pTextRenderer->DrawText(std::to_string(Frame::RadToDeg(interface->direction)), p1);
			if(bPutFinished) {
				continue;
			}

			const bool bMouseOn = pInterfaceMouseOn == interface;

			if(m_bMBLeftHolding && bMouseOn && m_bInterfaceCanPut) {
				if(CEditorDeviceComponent * pEDComp = Put(* interface)) {

					//////////// 检查新放置的装置的接口与此前已有接口的距离，若距离过近则进行连接
					
					using SInterface = CEditorDeviceComponent::SInterface;

					std::vector<SInterface *> v;
					pEDComp->GetAvailableInterfaces(& v);
					for(SInterface * & newerInterface : v) {
						for(const SInterfaceSet & olderInterfaceSet : m_interfaces) {
							for(SInterface * const & olderInterface : olderInterfaceSet.interfaces) {
								if(PointDistance(newerInterface->pos, olderInterface->pos) > __INTERFACE_CONNECT_DISTANCE)
									continue;
								if(CEditorDeviceComponent::ConnectInterfaces(newerInterface, olderInterface)) {
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
		// ------
		RegenerateInsBuffers();
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
		if((* it)->IsCabin()) {
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
	Remove(itWillBeErase);
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
}

void CEditorComponent::UpdateDevicesColor() {
	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->UpdateColor(GetCurrentColorSet());
	}
	RegenerateInsBuffers();
}

void CEditorComponent::ControllerBegin() {
	
}

void CEditorComponent::ControllerEnd() {
	
}

void CEditorComponent::FindAvailableInterfaces() {
	for(auto & pEDComp : m_editorDeviceComponents) {
		using SInterface = CEditorDeviceComponent::SInterface;

		std::vector<SInterface *> interfaces;
		pEDComp->GetAvailableInterfaces(& interfaces);

		for(SInterface * & interface : interfaces) {
			bool bNeedsToCreateANewInterfaceSet = true;
			for(SInterfaceSet & existingInterfaceSet : m_interfaces) {
				if(Frame::PointInRectangle(interface->pos, existingInterfaceSet.pos - __INTERFACESET_BUTTON_SIZE_HALF - 4.f, existingInterfaceSet.pos + __INTERFACESET_BUTTON_SIZE_HALF + 4.f)) {
					existingInterfaceSet.interfaces.push_back(interface);
					bNeedsToCreateANewInterfaceSet = false;
					break;
				}
			}
			if(bNeedsToCreateANewInterfaceSet) {
				m_interfaces.push_back({ { interface }, interface->pos + Frame::Vec2 { __INTERFACESET_BUTTON_SIZE_HALF, 0.f }.GetRotated(-interface->direction) });
			}
		}
	}
}

void CEditorComponent::ClearAvailableInterfaces() {
	m_pInterfaceMouseOn = nullptr;
	m_interfaces.clear();
}

void CEditorComponent::RefreshInterfaceCanPut(const CEditorDeviceComponent::SInterface & interfaceMouseOn) {
	if(Frame::CEntity * pEnt = Frame::gEntitySystem->SpawnEntity()) {
		if(CColliderComponent * pComp = pEnt->CreateComponent<CColliderComponent>()) {
			GetEditorDeviceRegistry()[GetEditorDeviceOrder()[m_pencilDeviceIndex]]->InitCollider(pComp, interfaceMouseOn.direction);
			pComp->GetEntity()->SetPosition(GetWillPutPos(interfaceMouseOn));
			m_bInterfaceCanPut = pComp->Collide().empty();
		}
		Frame::gEntitySystem->RemoveEntity(pEnt->GetId());
	}
}

Frame::Vec2 CEditorComponent::GetWillPutPos(const CEditorDeviceComponent::SInterface & interface, const Frame::GUID & willPutEditorDeviceGUID) const {
	return interface.pos + (Frame::Vec2 { GetEditorDeviceConfig(willPutEditorDeviceGUID).size.x * .5f, 0.f }).GetRotated(-interface.direction);
}

#if 0
struct SPipeInterface {
	CEditorDeviceComponent * pEditorDeviceComponent = nullptr;
	Frame::Vec2 pos {};
	int dirIndex = 0;
};

static inline std::vector<SPipeInterface> GetPipeInterfaces(CEditorDeviceComponent * pEditorDeviceComp) {
	IDeviceData::EType deviceType = pEditorDeviceComp->GetDeviceType();
	if(deviceType == IDeviceData::Unset) {
		return {};
	}

	int deviceDirIndex = pEditorDeviceComp->GetDirIndex();

	std::vector<SPipeInterface> results;

#define __FORMULA(dirIndex) { \
	Frame::Vec2 pos = pEditorDeviceComp->GetEntity()->GetPosition() + GetRectangleEdgePosByDirIndex(GetDevicePixelSize(deviceType), deviceDirIndex, dirIndex); \
	results.push_back({ pEditorDeviceComp, pos, dirIndex }); \
}

	switch(deviceType) {
	case IDeviceData::Engine:
		for(int i = 0; i < 4; i++) {
			__FORMULA(i)
		}
		break;
	case IDeviceData::Propeller:
	case IDeviceData::JetPropeller:
		__FORMULA(GetRevDirIndex(deviceDirIndex))
		break;
	}

#undef __FORMULA

	return results;
}

std::unordered_map<const CEditorDeviceComponent *, int> __map_PipeConnectableEDComp_MachinePartIndex;

static inline void RegenerateMachinePartIndices_For_EDCompsHavePipeInterface(const std::unordered_set<CEditorDeviceComponent *> & EDComps) {
	std::unordered_set<const CEditorDeviceComponent *> ignoreDevices;
	std::queue<const CEditorDeviceComponent *> nextJoints;
	std::queue<const CEditorDeviceComponent *> nexts;

	for(auto & ed : EDComps) {
		if(ed->GetDeviceType() == IDeviceData::Cabin) {
			nextJoints.push(ed);
			break;
		}
	}

	int currentMachinePartIndex = 0;

	std::function<void (const CEditorDeviceComponent *)> machinePartHandle = [&](const CEditorDeviceComponent * pEDComp) {
		if(!pEDComp || ignoreDevices.find(pEDComp) != ignoreDevices.end()) {
			return;
		}

		ignoreDevices.insert(pEDComp);

		if(pEDComp->IsMachinePartJoint()) {
			for(auto & neighbor : pEDComp->m_connectingEditorDevices) {
				nextJoints.push(neighbor);
			}
			return;
		}
		
		__map_PipeConnectableEDComp_MachinePartIndex.insert({ pEDComp, currentMachinePartIndex });

		for(const auto & pNeighborEDComp : pEDComp->m_connectingEditorDevices) {
			nexts.push(pNeighborEDComp);
		}

		return;
		};
	
	while(!nextJoints.empty()) {
		nexts.push(nextJoints.front());
		nextJoints.pop();
		while(!nexts.empty()) {
			machinePartHandle(nexts.front());
			nexts.pop();
		}
		currentMachinePartIndex++;
	}
}

static inline bool InTheSameMachinePart(const CEditorDeviceComponent * pEDComp1, const CEditorDeviceComponent * pEDComp2) {
	if(auto it1 = __map_PipeConnectableEDComp_MachinePartIndex.find(pEDComp1), it2 = __map_PipeConnectableEDComp_MachinePartIndex.find(pEDComp2);
		it1 != __map_PipeConnectableEDComp_MachinePartIndex.end() && it2 != __map_PipeConnectableEDComp_MachinePartIndex.end()
		) {
		if(it1->second == it2->second) {
			return true;
		}
	}
	return false;
}

// 如果无法连接到任何装置，返回的 pair 里的结构体将会都是默认值，也就是说结构体里的 pEditorDeviceComponent 将会是 nullptr.
static inline std::pair<SPipeInterface, SPipeInterface> GetNearestPipeInterfacesToEngineDevice(CEditorDeviceComponent * currEditorDeviceComp, const std::unordered_set<CEditorDeviceComponent *> & editorDeviceComponents) {
	float nearestDistance = 9999999.f;
	std::pair<SPipeInterface, SPipeInterface> nearestPipeInterfaces;

	std::vector<SPipeInterface> currEditorDevicePipeInterfaces = GetPipeInterfaces(currEditorDeviceComp);

	for(const auto & pEditorDeviceComp : editorDeviceComponents) {
		if(currEditorDeviceComp == pEditorDeviceComp || pEditorDeviceComp->GetDeviceType() != IDeviceData::EType::Engine || !InTheSameMachinePart(currEditorDeviceComp, pEditorDeviceComp)) {
			continue;
		}
		std::vector<SPipeInterface> pipeInterfaces = GetPipeInterfaces(pEditorDeviceComp);
		for(auto & pipeInterface : pipeInterfaces) {
			for(auto & currPipeInterface : currEditorDevicePipeInterfaces) {

				if(float distance = PointDistance(currPipeInterface.pos, pipeInterface.pos); distance < nearestDistance) {
					nearestDistance = distance;
					
					nearestPipeInterfaces.first = currPipeInterface;
					nearestPipeInterfaces.second = pipeInterface;
				}

			}
		}
	}

	return nearestPipeInterfaces;
}

// 重要！新增的 vector 的前两个元素将会为 起始节点 和 结束节点
static inline void PipeConnect(const std::pair<SPipeInterface, SPipeInterface> & interfaces, std::vector<std::vector<std::shared_ptr<SEditorPipeNode>>> & outToPushBack) {
	if(!interfaces.first.pEditorDeviceComponent || !interfaces.second.pEditorDeviceComponent) {
		return;
	}

	/*
	* 从两端同时出发
	* 朝向对方的方向，互相步进，寻找对方
	* 当处在同一横线或竖线上的时候，连接彼此
	*/

	const float stepLength = 32.f;
	const float targetDir[2] = {
		-(interfaces.second.pos - interfaces.first.pos).Degree(),
		-(interfaces.first.pos - interfaces.second.pos).Degree()
	};

	std::vector<std::shared_ptr<SEditorPipeNode>> pipe {
		std::make_shared<SEditorPipeNode>(interfaces.first.pos),
		std::make_shared<SEditorPipeNode>(interfaces.second.pos)
	};
	pipe[0]->dirIndexForDevice = interfaces.first.dirIndex;
	pipe[0]->pDevice = interfaces.first.pEditorDeviceComponent;
	pipe[1]->dirIndexForDevice = interfaces.second.dirIndex;
	pipe[1]->pDevice = interfaces.second.pEditorDeviceComponent;

	int nodesMovingDirIndex[2] = { interfaces.first.dirIndex, interfaces.second.dirIndex };

	// 若两节点相邻，直接连接上
	if(PointDistance(pipe[0]->pos, pipe[1]->pos) <= 33.f) {
		pipe[0]->nodes[nodesMovingDirIndex[0]] = pipe[1].get();
		pipe[1]->nodes[nodesMovingDirIndex[1]] = pipe[0].get();
		outToPushBack.push_back(pipe);
		return;
	}

	SEditorPipeNode * prevNode[2] { pipe[0].get(), pipe[1].get() };

	SEditorPipeNode currentStep[2] { { interfaces.first.pos }, { interfaces.second.pos } };
	currentStep[0].pos += GetDirPosAdd(nodesMovingDirIndex[0]) * stepLength;
	currentStep[1].pos += GetDirPosAdd(nodesMovingDirIndex[1]) * stepLength;

	static auto appendNewNode = [&](int turn) -> SEditorPipeNode * {
		auto pNewNode = std::make_shared<SEditorPipeNode>(currentStep[turn]);
		pipe.push_back(pNewNode);
		auto pNewNodeRaw = pNewNode.get();
		pNewNode->nodes[GetRevDirIndex(nodesMovingDirIndex[turn])] = prevNode[turn];
		prevNode[turn]->nodes[nodesMovingDirIndex[turn]] = pNewNodeRaw;
		prevNode[turn] = pNewNodeRaw;
		return pNewNodeRaw;
		};

	int whoseTurn = 0;
	for(int safe = static_cast<int>((currentStep[0].pos - currentStep[1].pos).Length() / stepLength) * 2; safe >= 0; safe--, whoseTurn = whoseTurn ? 0 : 1) {

		{
			int nextDirIndex = nodesMovingDirIndex[whoseTurn];
			float maxCos = -1.f;
			for(int i = 0; i < 4; i++) {
				if(i == GetRevDirIndex(nodesMovingDirIndex[whoseTurn])) {
					continue;
				}
				float currCos = cos(Frame::DegToRad(GetDegreeByDirIndex(i) - targetDir[whoseTurn]));
				if(currCos > maxCos) {
					maxCos = currCos;
					nextDirIndex = i;
				}
			}
			if(nodesMovingDirIndex[whoseTurn] != nextDirIndex) {
				appendNewNode(whoseTurn);

				nodesMovingDirIndex[whoseTurn] = nextDirIndex;
			}
		}

		currentStep[whoseTurn].pos += GetDirPosAdd(nodesMovingDirIndex[whoseTurn]) * stepLength;
		
		// 当处在同一横线或竖线上的时候，连接彼此
		if(abs(currentStep[0].pos.x - currentStep[1].pos.x) <= stepLength - 1.f) {
			currentStep[0].pos.x = currentStep[1].pos.x;

			SEditorPipeNode * pNodeA = appendNewNode(0);
			SEditorPipeNode * pNodeB = appendNewNode(1);
			int dirIndexFromAToB = GetDirIndexByDegree(-(pNodeB->pos - pNodeA->pos).Degree());
			pNodeA->nodes[dirIndexFromAToB] = pNodeB;
			pNodeB->nodes[GetRevDirIndex(dirIndexFromAToB)] = pNodeA;

			break;
		} else
		if(abs(currentStep[0].pos.y - currentStep[1].pos.y) <= stepLength - 1.f) {
			currentStep[0].pos.y = currentStep[1].pos.y;

			SEditorPipeNode * pNodeA = appendNewNode(0);
			SEditorPipeNode * pNodeB = appendNewNode(1);
			int dirIndexFromAToB = GetDirIndexByDegree(-(pNodeB->pos - pNodeA->pos).Degree());
			pNodeA->nodes[dirIndexFromAToB] = pNodeB;
			pNodeB->nodes[GetRevDirIndex(dirIndexFromAToB)] = pNodeA;

			break;
		}
	}

	// 移除多余的中间结点
	for(auto it = pipe.begin(); it != pipe.end();) {
		auto & pNode = * it;
		if(pNode->nodes[0] && pNode->nodes[2] && !pNode->nodes[1] && !pNode->nodes[3]) {
			pNode->nodes[0]->nodes[2] = pNode->nodes[2];
			pNode->nodes[2]->nodes[0] = pNode->nodes[0];
			it = pipe.erase(it);
		} else
		if(pNode->nodes[1] && pNode->nodes[3] && !pNode->nodes[0] && !pNode->nodes[2]) {
			pNode->nodes[1]->nodes[3] = pNode->nodes[3];
			pNode->nodes[3]->nodes[1] = pNode->nodes[1];
			it = pipe.erase(it);
		} else {
			it++;
		}
	}

	outToPushBack.push_back(pipe);
}
#endif

CEditorDeviceComponent * __Put(std::unordered_set<CEditorDeviceComponent *> & m_editorDeviceComponents, const SColorSet & colorSet, const Frame::Vec2 & pos, const Frame::GUID & guid, float rotation, bool ignoreCollision) {
	
	// TODO - ignoreCollision
	ignoreCollision;

	if(Frame::CEntity * pEntity = Frame::gEntitySystem->SpawnEntity()) {
		pEntity->SetPosition(pos);
		CEditorDeviceComponent * pEDComp = pEntity->CreateComponent<CEditorDeviceComponent>();
		if(pEDComp->Initialize(guid, rotation)) {
			m_editorDeviceComponents.insert(pEDComp);
			pEDComp->UpdateColor(colorSet);
			return pEDComp;
		} else {
			Frame::gEntitySystem->RemoveEntity(pEntity->GetId());
		}
	}
	return nullptr;
}

CEditorDeviceComponent * CEditorComponent::Put(const CEditorDeviceComponent::SInterface & interface) {
	return __Put(m_editorDeviceComponents, GetCurrentColorSet(), GetWillPutPos(interface), GetEditorDeviceOrder()[m_pencilDeviceIndex], -interface.direction, false);
}

void CEditorComponent::Remove(decltype(m_editorDeviceComponents.begin()) it, bool regenerateInsBuffers) {
	if(it != m_editorDeviceComponents.end()) {
		Frame::gEntitySystem->RemoveEntity((* it)->GetEntity()->GetId());
		m_editorDeviceComponents.erase(it);

		if(regenerateInsBuffers) RegenerateInsBuffers();
	}
}

#if 0
void CEditorComponent::RegenerateAllPipes() {
	RegenerateMachinePartIndices_For_EDCompsHavePipeInterface(m_editorDeviceComponents);

	m_pipes.clear();
	
	for(auto & ed : m_editorDeviceComponents) {
		if(!IsDeviceHasPipeInterface(ed->GetDeviceType())) {
			continue;
		}

		PipeConnect(GetNearestPipeInterfacesToEngineDevice(ed, m_editorDeviceComponents), m_pipes);
	}
}

void CEditorComponent::RegenerateNearPipes(CEditorDeviceComponent * pEDComp) {
	RegenerateMachinePartIndices_For_EDCompsHavePipeInterface(m_editorDeviceComponents);

	std::vector<CEditorDeviceComponent *> devicesConnectable;

	for(auto & ed : m_editorDeviceComponents) {
		if(!IsDeviceHasPipeInterface(ed->GetDeviceType()) || !InTheSameMachinePart(pEDComp, ed)) {
			continue;
		}
		devicesConnectable.push_back(ed);
	}

	Frame::Vec2 myPos = pEDComp->GetEntity()->GetPosition();

	std::sort(devicesConnectable.begin(), devicesConnectable.end(), [& myPos](const CEditorDeviceComponent * p1, const CEditorDeviceComponent * p2) {
		return PointDistance(myPos, p1->GetEntity()->GetPosition()) < PointDistance(myPos, p2->GetEntity()->GetPosition());
		});

	int count = 8;
	for(auto & ed : devicesConnectable) {
		if(count-- <= 0) {
			break;
		}

		for(auto it = m_pipes.begin(); it != m_pipes.end(); it++) {
			if((* it)[0]->pDevice == ed) {
				m_pipes.erase(it);
				PipeConnect(GetNearestPipeInterfacesToEngineDevice(ed, m_editorDeviceComponents), m_pipes);
				break;
			}
		}
	}

	PipeConnect(GetNearestPipeInterfacesToEngineDevice(pEDComp, m_editorDeviceComponents), m_pipes);
}
#endif

void CEditorComponent::DrawDevicePreview(size_t editorDeviceIndex, const Frame::Vec2 & pos, float alpha, float rot, float scale, Frame::ColorRGB customColor, bool useCustomColor) const {

	SColorSet colorSet = GetCurrentColorSet();
	if(useCustomColor) {
		colorSet.color1 = customColor;
		colorSet.color2 = customColor;
	}

	GetEditorDeviceRegistry()[GetEditorDeviceOrder()[editorDeviceIndex]]->DrawPreview(pos, colorSet, alpha, scale, rot);

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
			return pEDComp && pEDComp->IsCabin();
		}
	);
	if(it == m_editorDeviceComponents.end()) {
		return;
	}
	if(auto pEnt = Frame::gEntitySystem->SpawnEntity()) {
		if(auto pComp = pEnt->CreateComponent<CMachineComponent>()) {
			pComp->Initialize(* it, GetCurrentColorSet());
			// TODO
		}
	}
}

void CEditorComponent::__RegenerateInsBuffers() {
	if(__regenerateInsBuffersDelay > 0) {
		__regenerateInsBuffersDelay--;
		return;
	}
	if(__regenerateInsBuffersDelay == 0) {
		__regenerateInsBuffersDelay = -1;
	} else {
		return;
	}

	m_insBuffers.clear();

	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->GetConnectorsRenderingInstanceData(m_insBuffers);
	}

	for(auto & pEDComp : m_editorDeviceComponents) {
		pEDComp->GetRenderingInstanceData(m_insBuffers);
	}
}

struct SSerializedEditorDevice {
	Frame::GUID guid;
	Frame::Vec2 position; // 相对于 驾驶舱 Cabin 来说的
	float rotation;       // 同上
	
	struct SConnection {
		size_t iEDComp; // 连接的装置的下标
		int myInterfaceID; // 哪个接口连接着 iEDComp 记录的装置
		int toInterfaceID; // 接口连接到的 iEDComp 记录的装置的接口，也就是 iEDComp 记录的装置的什么接口连接着自己
	};
	std::vector<SConnection> connections;
};

std::string CEditorComponent::SerializeEditorMachine() const {
	std::vector<std::pair<const CEditorDeviceComponent *, SSerializedEditorDevice>> v;
	const CEditorDeviceComponent * pCabin = nullptr;

	// 找出 Cabin
	for(const auto & pEDComp : m_editorDeviceComponents) {
		if(pEDComp->IsCabin()) {
			pCabin = pEDComp;
			break;
		}
	}

	if(!pCabin) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "Try to serialize editor machine, but cannot find Cabin.");
		return "";
	}

	const Frame::Vec2 cabinPos = pCabin->GetEntity()->GetPosition();
	const float cabinRot = pCabin->GetEntity()->GetRotation();

	// 将每个编辑器装置转换成 SSerializedEditorDevice
	for(const auto & pEDComp : m_editorDeviceComponents) {
		std::shared_ptr<IEditorDeviceData> pData = pEDComp->GetData();
		if(!pData) {
			continue;
		}
		SSerializedEditorDevice current;
		current.guid = pData->GetConfig().guid;
		current.position = pEDComp->GetEntity()->GetPosition() - cabinPos;
		current.rotation = pEDComp->GetEntity()->GetRotation() - cabinRot;

		// 只记录自己与自己之前的装置之间的连接关系，确保每个连接关系只被记录一次
		for(size_t i = 0, len = v.size(); i < len; i++) {
			auto pEDCompBeforeThis = v[i].first;
			for(const auto & interface : pEDComp->m_interfaces) {
				if(!interface.to || interface.to->from != pEDCompBeforeThis) {
					continue;
				}
				SSerializedEditorDevice::SConnection connection;
				connection.iEDComp = i;
				connection.myInterfaceID = interface.ID;
				connection.toInterfaceID = interface.to->ID;
				current.connections.push_back(connection);
			}
		}

		v.push_back({ pEDComp, current });
	}

	// 最终转换成 JSON
	nlohmann::json json = nlohmann::json::array();
	for(const auto & [_, current] : v) {
		nlohmann::json currConnectionsJson = nlohmann::json::array();
		for(const auto & connection : current.connections) {
			currConnectionsJson.push_back({
				{ "to", connection.iEDComp },
				{ "ID", connection.myInterfaceID },
				{ "toID", connection.toInterfaceID }
				});
		}
		nlohmann::json currJson = {
			{ "guidhigh", current.guid.high },
			{ "guidlow", current.guid.low },
			{ "x", current.position.x },
			{ "y", current.position.y },
			{ "rot", current.rotation },
			{ "connections", currConnectionsJson }
		};
		json.push_back(currJson);
	}
	return json.dump();
}

void CEditorComponent::DeserializeEditorMachine(std::string_view str) {
	std::vector<std::pair<CEditorDeviceComponent *, SSerializedEditorDevice>> v;
	
	// 清除所有的编辑器装置
	while(m_editorDeviceComponents.size()) {
		Remove(m_editorDeviceComponents.begin(), false);
	}
	RegenerateInsBuffers();

	// 解析 JSON
	try {
		nlohmann::json json = nlohmann::json::parse(str);
		for(const auto & currJson : json) {
			SSerializedEditorDevice current;
			current.guid.high = currJson["guidhigh"];
			current.guid.low = currJson["guidlow"];
			current.position.x = currJson["x"];
			current.position.y = currJson["y"];
			current.rotation = currJson["rot"];
			for(const auto & currConnectionsJson : currJson["connections"]) {
				SSerializedEditorDevice::SConnection connection;
				connection.iEDComp = currConnectionsJson["to"];
				connection.myInterfaceID = currConnectionsJson["ID"];
				connection.toInterfaceID = currConnectionsJson["toID"];
				current.connections.push_back(connection);
			}
			v.push_back({ nullptr, current });
		}
	} catch(const nlohmann::json::exception & e) {
		Frame::Log::Log(Frame::Log::ELevel::Error, "DeserializeEditorMachine(): Illegal JSON: %s", e.what());
	}

	// 生成解析出的编辑器装置
	for(auto & [pEDComp, serialized] : v) {
		pEDComp = __Put(m_editorDeviceComponents, GetCurrentColorSet(), serialized.position, serialized.guid, serialized.rotation, true);
		for(const auto & connection : serialized.connections) {
			CEditorDeviceComponent::ConnectInterfaces(pEDComp, connection.myInterfaceID, v[connection.iEDComp].first, connection.toInterfaceID);
		}
	}
}

#undef __DRAW_TEXT_BUTTON
