#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameMath/ColorMath.h>

#include "../../EditorDevicesData.h"
#include "../../Assets.h"
#include "../../Utility.h"
#include "../../Texts.h"
#include "../../Controller.h"
#include "../../GUI/GUI.h"

#include "EditorDeviceComponent.h"

#include <vector>
#include <unordered_set>
#include <memory>

class CEditorDeviceComponent;
class CCameraComponent;

struct SEditorPipeNode;

class CEditorComponent final : public Frame::IEntityComponent {
public:

	static CEditorComponent * s_pEditorComponent;

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{6B9BDDAF-A846-4013-AC33-5CEF1960B24A}");
	}

	virtual void Initialize() override;
	virtual void OnShutDown() override;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

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

	void SwitchTool(ETool tool);

	size_t m_pencilDeviceIndex; // 使用铅笔工具时，选中的要绘制的装置

	void SwitchPencilDevice(size_t index) {
		size_t size = GetEditorDeviceOrder().size();
		m_pencilDeviceIndex = size == 0 ? 0 : (index >= size ? size - 1 : index);
		m_pInterfaceMouseOn = nullptr;
	}

	void PencilPrevDevice() {
		size_t deviceInd = m_pencilDeviceIndex, i = 0, size = GetEditorDeviceOrder().size();
		for(; i < size; i++) {
			deviceInd = deviceInd <= 0 ? size - 1 : deviceInd - 1;
			if(GetEditorDeviceConfig(GetEditorDeviceOrder()[deviceInd]).pencilEnable) {
				break;
			}
		}
		if(i == size) {
			return;
		}
		SwitchPencilDevice(deviceInd);
	}
	void PencilNextDevice() {
		size_t deviceInd = m_pencilDeviceIndex, i = 0, size = GetEditorDeviceRegistry().size();
		for(; i < size; i++) {
			deviceInd = deviceInd >= size - 1 ? 0 : deviceInd + 1;
			if(GetEditorDeviceConfig(GetEditorDeviceOrder()[deviceInd]).pencilEnable) {
				break;
			}
		}
		if(i == size) {
			return;
		}
		SwitchPencilDevice(deviceInd);
	}

	auto GetPencilDeviceData() const -> decltype(GetEditorDeviceData("")) {
		return GetEditorDeviceData(GetEditorDeviceOrder()[m_pencilDeviceIndex]);
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
	/*struct SInterface {
		SInterface(CEditorDeviceComponent * _pEditorDeviceComponent, const Frame::Vec2 & _pos, int _directionIndex)
			: pEditorDeviceComponent { _pEditorDeviceComponent }
			, pos { _pos }
			, directionIndex { _directionIndex }
		{}
		CEditorDeviceComponent * pEditorDeviceComponent;
		Frame::Vec2 pos;
		int directionIndex;
	};*/

private:

	// 铅笔工具，一个橙色按钮上可能会有多个方向的不同接口
	// 一个 SInterfaceSet 就表示一个橙色按钮，里面 interfaces[i] 就是一个接口
	struct SInterfaceSet {
		std::vector<CEditorDeviceComponent::SInterface *> interfaces;
		Frame::Vec2 pos;
	};

	std::vector<SInterfaceSet> m_interfaces;

	const CEditorDeviceComponent::SInterface * m_pInterfaceMouseOn = nullptr;
	bool m_bInterfaceCanPut = false;

	void FindAvailableInterfaces();
	void ClearAvailableInterfaces();

	void RefreshInterfaceCanPut(const CEditorDeviceComponent::SInterface & interfaceMouseOn);

	bool ConnectDevices(CEditorDeviceComponent::SInterface * interface1, CEditorDeviceComponent::SInterface * interface2) const {
		if(!interface1 || !interface2 || !interface1->from || !interface2->from) {
			return false;
		}
		if(interface1->to || interface2->to) {
			return false;
		}
		interface1->to = interface2->from;
		interface2->to = interface1->from;
		return true;
	}

	/* -------------------- 装置放置 -------------------- */

	Frame::Vec2 GetWillPutPos(const CEditorDeviceComponent::SInterface & interface) const {
		const auto & order = GetEditorDeviceOrder();
		if(order.size() <= m_pencilDeviceIndex) {
			return {};
		}
		return GetWillPutPos(interface, order[m_pencilDeviceIndex]);
	}
	Frame::Vec2 GetWillPutPos(const CEditorDeviceComponent::SInterface & interface, const Frame::GUID & willPutEditorDeviceGUID) const;

	CEditorDeviceComponent * Put(const CEditorDeviceComponent::SInterface & interface);

	/* -------------------- 管道接口（插槽） -------------------- */



	/* -------------------- 管道节点 -------------------- */

	std::vector<std::vector<std::shared_ptr<SEditorPipeNode>>> m_pipes;

	void RegenerateAllPipes();
	void RegenerateNearPipes(CEditorDeviceComponent * pEDComp);

	/* -------------------- 其它绘制函数 -------------------- */

	void DrawDevicePreview(size_t editorDeviceIndex, const Frame::Vec2 & pos, float alpha, float rot, float scale) const {
		DrawDevicePreview(editorDeviceIndex, pos, alpha, rot, scale, 0, false);
	}
	void DrawDevicePreview(size_t editorDeviceIndex, const Frame::Vec2 & pos, float alpha, float rot, float scale, Frame::ColorRGB customColor, bool useCustomColor = true) const;

	/* -------------------- 未归类 -------------------- */

	void ButtonEnd(const Frame::Vec2 & rightBottom);

	void SummonMachine();

	std::vector<Frame::CRenderer::SInstanceBuffer> m_insBuffers;
	int __regenerateInsBuffersDelay = 0;
	void RegenerateInsBuffers() {
		__regenerateInsBuffersDelay = 1;
	}
	void __RegenerateInsBuffers();

};
