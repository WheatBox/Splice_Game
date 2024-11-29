#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameMath/ColorMath.h>

#include "../../DevicesData.h"
#include "../../Assets.h"
#include "../../Utility.h"
#include "../../Texts.h"
#include "../../Controller.h"
#include "../../GUI/GUI.h"

#include "EditorControllerGUI.h"

#include <vector>
#include <unordered_set>
#include <memory>

class CEditorDeviceComponent;
class CCameraComponent;

struct SEditorPipeNode;

class CEditorComponent final : public Frame::IEntityComponent {
public:

	static CEditorComponent * s_pEditorComponent;

	virtual void Initialize() override;
	virtual void OnShutDown() override;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CEditorComponent> type) {
		type.SetGUID("{6B9BDDAF-A846-4013-AC33-5CEF1960B24A}");
	}

	void SetWorking(bool b);
	bool GetWorking() const {
		return m_bWorking;
	}

private:
	bool m_bWorking = true;

	/* -------------------- 该组件之外的资源 -------------------- */

	Frame::CFont * m_pFont = nullptr;

	CCameraComponent * m_pCameraComponent = nullptr;

	std::unordered_set<CEditorDeviceComponent *> m_editorDeviceComponents;

	Frame::CEntity * m_pDeviceConnectorRendererEntity = nullptr;

	/* -------------------- GUI -------------------- */

	void InitGUI();
	void InitGUI_Toolbar();
	void InitGUI_ToolPencilMenu();
	void InitGUI_ToolSwatchesMenu();
	void InitGUI_ToolSwatchesColorEditor();
	void InitGUI_ToolControllerMenu();
	void InitGUI_ToolControllerController();
	void InitGUI_OperationPrompt();

	std::shared_ptr<GUI::CGUI> m_pGUI {};
	std::shared_ptr<GUI::CDraggablePage> m_pToolPencilMenu {};
	std::shared_ptr<GUI::CDraggablePage> m_pToolSwatchesMenu {};
	std::shared_ptr<GUI::CDraggablePage> m_pToolSwatchesColorEditor {};
	std::shared_ptr<GUI::CDraggablePage> m_pToolControllerMenu {};
	std::shared_ptr<CEditorControllerPage> m_pToolControllerController {};

	/* -------------------- 鼠标 -------------------- */

	bool m_bMouseOnGUI = false;

	bool m_bMBLeftPressed = false;
	bool m_bMBLeftHolding = false;
	bool m_bMBLeftReleased = false;
	bool m_bMBRightPressed = false;

	void SetMouseLabel(Texts::EText text) {
		m_mouseLabelText = text;
	}
	Texts::EText m_mouseLabelText = Texts::EText::EMPTY;

	/* -------------------- 工具菜单 -------------------- */

	static constexpr float toolbarWidth = 48.f;

	enum class ETool {
		Hand,
		Pencil,
		Eraser,
		Swatches,
		Controller
	} m_tool = ETool::Hand;

	static constexpr float GetToolMenuWidth(ETool tool) {
		constexpr float toolMenuWidth[] = {
			0.f,
			128.f,
			0.f,
			160.f,
			128.f
		};
		return toolMenuWidth[static_cast<int>(tool)];
	}

	void RenderAndProcessControllerMenu(const Frame::Vec2 & leftTopPos);

	/* -------------------- 工具运行 -------------------- */

	void Pencil();
	void Eraser();

	IDeviceData::EType m_pencilDevice = IDeviceData::Unset; // 使用铅笔工具时，选中的要绘制的装置

	void SwitchTool(ETool tool);
	void SwitchPencilDevice(IDeviceData::EType device) {
		m_pencilDevice = device;
		m_pInterfaceMouseOn = nullptr;
	}

	/* -------------------- 调色盘工具杂项 -------------------- */

	std::vector<SColorSet> m_colorSets {
		{ 0x554D46, 0x732520, 0xA24E46, 0xBD4700 },
		{ 0xA62520, 0x898784, 0x292328, 0x914C4D },
		{ 0xCCCC00, 0x00CCCC, 0xCC00CC, 0x777777 },
		{ 0xCC0000, 0x00CC00, 0x0000CC, 0x777777 }
	};
	size_t m_currColorSetIndex = 0;

	SColorSet m_colorSetEditing {};
	Frame::ColorRGB SColorSet::* m_pColorSetEditingColor = & SColorSet::color1;

public:
	SColorSet & GetColorSetEditing() {
		return m_colorSetEditing;
	}

	void SetColorSetEditingColor(Frame::ColorRGB SColorSet::* memberVarInColorSet) {
		m_pColorSetEditingColor = memberVarInColorSet;
	}

	Frame::ColorRGB SColorSet::* GetColorSetEditingColor() const {
		return m_pColorSetEditingColor;
	}

private:
	const SColorSet & GetCurrentColorSet() const {
		static SColorSet defaultRes {};
		if(m_currColorSetIndex >= m_colorSets.size()) {
			return defaultRes;
		}
		return m_colorSets[m_currColorSetIndex];
	}

	void SetCurrentColorSetByIndex(size_t colorSetIndex) {
		if(colorSetIndex >= m_colorSets.size()) {
			return;
		}
		m_currColorSetIndex = colorSetIndex;
		m_colorSetEditing = GetCurrentColorSet();
		UpdateDevicesColor();
	}

	void ApplyCurrentColorSet() {
		if(m_currColorSetIndex >= m_colorSets.size()) {
			return;
		}
		m_colorSets[m_currColorSetIndex] = m_colorSetEditing;
		UpdateDevicesColor();
	}

	void UpdateDevicesColor();

	/* -------------------- 控制器编辑工具杂项 -------------------- */

	void SwitchControllerPencilElement(Controller::EElement elem) {
		m_controllerPencilElement = elem;
	}

	Controller::EElement m_controllerPencilElement = Controller::EElement::Unknown;

	// ---

	Controller::SController m_controllerEditing {};
	
	CMenuDragger m_controllerMenuDragger {};

	std::shared_ptr<Controller::IElement> m_pDraggingControllerElement { nullptr };
	Frame::Vec2 m_draggingControllerElementPosRelativeToMouse {};

	bool m_bControllerResizing = false;
	Frame::Vec2i m_controllerResizingMinSize {};

	// ---

	struct SToolControllerStuff {
		std::unordered_set<CEditorDeviceComponent *> engineEDComps;
		std::unordered_set<CEditorDeviceComponent *> highlightEDComps; // 鼠标悬浮在某个引擎装置上的时候，高亮显示所有与之有管道相连的其它装置
		std::unordered_set<size_t> highlightPipeIndices;
		CEditorDeviceComponent * pEDCompWaitingForKey = nullptr;
	} m_toolControllerStuff;

	void ControllerBegin();
	void ControllerEnd();

	/* -------------------- 装置接口（插槽） -------------------- */

public:
	struct SInterface {
		SInterface(CEditorDeviceComponent * _pEditorDeviceComponent, const Frame::Vec2 & _pos, int _directionIndex)
			: pEditorDeviceComponent { _pEditorDeviceComponent }
			, pos { _pos }
			, directionIndex { _directionIndex }
		{}
		CEditorDeviceComponent * pEditorDeviceComponent;
		Frame::Vec2 pos;
		int directionIndex;
	};

private:

	struct SInterfaceSet {
		std::vector<SInterface> interfaces;
		Frame::Vec2 pos;
	};

	std::vector<SInterfaceSet> m_interfaces;

	const SInterface * m_pInterfaceMouseOn = nullptr;
	bool m_bInterfaceCanPut = false;

	void FindAvailableInterfaces();
	void ClearAvailableInterfaces();

	void RefreshInterfaceCanPut(const SInterface & interfaceMouseOn);

	/* -------------------- 装置放置 -------------------- */

	Frame::Vec2 GetWillPutPos(const SInterface & interface) const;

	CEditorDeviceComponent * Put(const SInterface & interface);
	// TODO - 隐去下面两个函数
	CEditorDeviceComponent * Put(const Frame::Vec2 & pos, int dirIndex) {
		return Put(pos, m_pencilDevice == IDeviceData::Unset ? IDeviceData::Shell : m_pencilDevice, dirIndex);
	}
	CEditorDeviceComponent * Put(const Frame::Vec2 & pos, IDeviceData::EType type, int dirIndex);

	/* -------------------- 管道接口（插槽） -------------------- */



	/* -------------------- 管道节点 -------------------- */

	std::vector<std::vector<std::shared_ptr<SEditorPipeNode>>> m_pipes;

	void RegenerateAllPipes();

	/* -------------------- 其它绘制函数 -------------------- */

	void DrawDevicePreview(IDeviceData::EType type, const Frame::Vec2 & pos, float alpha, int dirIndex, float scale) const {
		DrawDevicePreview(type, pos, alpha, dirIndex, scale, 0, false);
	}
	void DrawDevicePreview(IDeviceData::EType type, const Frame::Vec2 & pos, float alpha, int dirIndex, float scale, Frame::ColorRGB customColor, bool useCustomColor = true) const;

	/* -------------------- 未归类 -------------------- */

	void ButtonEnd(const Frame::Vec2 & rightBottom);

	void SummonMachine();

};