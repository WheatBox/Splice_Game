#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameMath/ColorMath.h>

#include "../../DevicesData.h"
#include "../../Assets.h"
#include "../../Utility.h"
#include "../../Pipe.h"
#include "../../Texts.h"
#include "../../Controller.h"

#include <vector>
#include <unordered_set>
#include <memory>

class CEditorDeviceComponent;
class CCameraComponent;

class CEditorComponent final : public Frame::IEntityComponent {
public:

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

	/* -------------------- 工具菜单 -------------------- */

	static constexpr float toolbarWidth = 48.f;

	enum class ETool {
		Hand,
		Pencil,
		Eraser,
		Pipe,
		Swatches,
		Controller
	} m_tool = ETool::Hand;

	static constexpr float GetToolMenuWidth(ETool tool) {
		constexpr float toolMenuWidth[] = {
			0.f,
			128.f,
			0.f,
			48.f,
			160.f,
			128.f
		};
		return toolMenuWidth[static_cast<int>(tool)];
	}

	void RenderAndProcessToolbar(const Frame::Vec2 & leftTopPos, const Frame::Vec2 & rightBottomPos);
	void RenderAndProcessPencilMenu(const Frame::Vec2 & leftTopPos);
	void RenderAndProcessPipeMenu(const Frame::Vec2 & leftTopPos);
	void RenderAndProcessSwatchesMenu(const Frame::Vec2 & leftTopPos);
	void RenderAndProcessControllerMenu(const Frame::Vec2 & leftTopPos);

	/* -------------------- 工具运行 -------------------- */

	void Pencil();
	void Eraser();
	void Pipe_PencilMode();
	void Pipe_EraserMode();
	void Pipe_InsertMode();
	void Controller();

	IDeviceData::EType m_pencilDevice = IDeviceData::Unset; // 使用铅笔工具时，选中的要绘制的装置

	enum class EPipeToolMode {
		Pencil,
		Eraser,
		Insert
	} m_pipeToolMode = EPipeToolMode::Pencil; // 使用管道工具时，选中的模式

	void SwitchTool(ETool tool);
	void SwitchPencilDevice(IDeviceData::EType device) {
		m_pencilDevice = device;
		m_pInterfaceMouseOn = nullptr;
	}
	void SwitchPipeToolMode(EPipeToolMode mode) {
		m_pipeToolMode = mode;
		PipeToolCleanUp();
	}

	/* -------------------- 调色盘工具杂项 -------------------- */

	std::vector<SColorSet> m_colorSets {
		{ 0x554D46, 0x732520, 0xA24E46, 0xBD4700 },
		{ 0xA62520, 0x898784, 0x292328, 0x914C4D },
		{ 0xCCCC00, 0x00CCCC, 0xCC00CC, 0x777777 },
		{ 0xCC0000, 0x00CC00, 0x0000CC, 0x777777 }
	};
	size_t m_currColorSetIndex = 0;

	int m_swatchesColorEditingIndex = 0;
	SColorSet m_colorSetEditing {};

	SColorSet GetCurrentColorSet() const {
		if(m_currColorSetIndex >= m_colorSets.size()) {
			return {};
		}
		return m_colorSets[m_currColorSetIndex];
	}

	void SynchCurrentColorSet() {
		m_colorSetEditing = GetCurrentColorSet();
	}
	void ApplyCurrentColorSet() {
		if(m_currColorSetIndex >= m_colorSets.size()) {
			return;
		}
		m_colorSets[m_currColorSetIndex] = m_colorSetEditing;
		UpdateDevicesColor();
	}

	void UpdateDevicesColor();

	CMenuDragger m_colorEditorMenuDragger {};

	/* -------------------- 控制器编辑工具杂项 -------------------- */

	Controller::EElement m_controllerPencilElement = Controller::EElement::Unknown;
	Controller::SController m_controllerEditing {};
	
	CMenuDragger m_controllerMenuDragger {};

	Frame::Vec2 m_draggingontrollerElementPosRelativeToMouse {};
	std::shared_ptr<Controller::IElement> m_pDraggingControllerElement { nullptr };

	// ---

	struct SToolControllerStuff {
		std::unordered_set<CEditorDeviceComponent *> engineEDComps;
		std::unordered_set<CEditorDeviceComponent *> highlightEDComps; // 鼠标悬浮在某个引擎装置上的时候，高亮显示所有与之有管道相连的其它装置
		std::unordered_set<size_t> highlightPipeIndices;
		CEditorDeviceComponent * pEDCompWaitingForKey = nullptr;
	} m_toolControllerStuff;

	void ControllerBegin();
	void ControllerEnd();

	/* -------------------- 管道工具杂项 -------------------- */

	struct SPipeInsertData {
		size_t pipeIndex = SIZE_MAX; // 管道工具插入模式，要插入的目标管道在 m_pipes 中的下标（注意不是管道节点的下标）
		size_t pipeEditingMinIndex = SIZE_MAX;
		bool isNewCross = false;
		std::unordered_set<CEditorDeviceComponent *> devicesThatHasAlreadyConnected;
	} m_pipeInsertData;

	void PipeToolCleanUp() {
		if(m_tool == ETool::Pipe && (m_pipeToolMode == EPipeToolMode::Pencil || m_pipeToolMode == EPipeToolMode::Insert)) {
			FindAvailablePipeInterfaces();
		}
		CancelPipeNodesEditing();
		m_pipeInsertData = SPipeInsertData {};
	}

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
	CEditorDeviceComponent * Put(const Frame::Vec2 & pos, int dirIndex) {
		return Put(pos, m_pencilDevice == IDeviceData::Unset ? IDeviceData::Shell : m_pencilDevice, dirIndex);
	}
	CEditorDeviceComponent * Put(const Frame::Vec2 & pos, IDeviceData::EType type, int dirIndex);

	/* -------------------- 管道接口（插槽） -------------------- */

public:
	typedef SInterface SPipeInterface;

private:
	std::vector<SPipeInterface> m_pipeInterfaces;

	size_t m_pipeInterfaceSelectingIndex = SIZE_MAX;

	void DeselectPipeInterface() {
		m_pipeInterfaceSelectingIndex = SIZE_MAX;
	}
	bool IsSelectingPipeInterface() const {
		return m_pipeInterfaceSelectingIndex < m_pipeInterfaces.size();
	}
	const SPipeInterface & GetSelectingPipeInterface() const {
		return m_pipeInterfaces[m_pipeInterfaceSelectingIndex];
	}

	void GetAvailablePipeInterfaces(std::vector<SPipeInterface> * outToPushBack, CEditorDeviceComponent * pEDComp) const;
	void FindAvailablePipeInterfaces();
	void FindAvailablePipeInterfacesMachinePart(SPipeInterface pipeInterface);
	void FindAvailablePipeInterfacesMachinePart(CEditorDeviceComponent * _pEDComp);

	/* -------------------- 管道节点 -------------------- */

	// 将一整条管道（包含各个小分支在内）视为一个整体进行处理
	// 此处的 m_pipes 存储的就是一个个上面这句话里提到的“整体”
	std::vector<std::vector<SEditorPipeNode *>> m_pipes;
	std::vector<SEditorPipeNode *> m_pipeNodesEditing;

	SEditorPipeNode * CreatePipeNodeByInterface(const SPipeInterface & interface) const;

	void ErasePipeNode(size_t pipeIndex, SEditorPipeNode * pipeNode);

	void BindPipeNodeWithEditorDeviceComponent(SEditorPipeNode * pPipeNode, CEditorDeviceComponent * pEDComp) const;
	void UnbindPipeNodeWithEditorDeviceComponent(SEditorPipeNode * pPipeNode) const;

	void DestroyPipeNode(SEditorPipeNode * pPipeNode) const {
		UnbindPipeNodeWithEditorDeviceComponent(pPipeNode);
		delete pPipeNode;
	}

	void MovePipeNodesEditingToPipeNodes() {
		if(m_pipeInsertData.pipeIndex == SIZE_MAX) {
			m_pipes.push_back(m_pipeNodesEditing);
		} else {
			m_pipeNodesEditing.swap(m_pipes[m_pipeInsertData.pipeIndex]);
		}
		m_pipeNodesEditing.clear();

		DeselectPipeInterface();
	}
	void CancelPipeNodesEditing() {
		if(m_pipeNodesEditing.size() == 0) {
			return;
		}
		if(m_pipeInsertData.pipeIndex != SIZE_MAX) {
			CancelPipeNodesInserting();
		} else {
			for(const auto & p : m_pipeNodesEditing) {
				DestroyPipeNode(p);
			}
			m_pipeNodesEditing.clear();
		}
	}
	void UndoPipeNodesEditing() {
		if(m_pipeNodesEditing.size() == 0) {
			return;
		}
		const auto & p = m_pipeNodesEditing.back();
		for(int i = 0; i < 4; i++) {
			if(p->nodes[i]) {
				for(int j = 0; j < 4; j++) {
					if(p->nodes[i]->nodes[j] == p) {
						p->nodes[i]->nodes[j] = nullptr;
						break;
					}
				}
			}
		}
		DestroyPipeNode(p);
		m_pipeNodesEditing.pop_back();
	}
	void CancelPipeNodesInserting() {
		if(m_pipeNodesEditing.size() == 0 || m_pipeInsertData.pipeEditingMinIndex == SIZE_MAX) {
			return;
		}
		for(size_t i = m_pipeInsertData.pipeEditingMinIndex + 1, len = m_pipeNodesEditing.size(); i < len; i++) {
			UndoPipeNodesEditing();
		}
		UndoNewCrossOfPipeNodesInserting();

		MovePipeNodesEditingToPipeNodes();
	}
	void UndoNewCrossOfPipeNodesInserting() {
		if(m_pipeInsertData.pipeIndex == SIZE_MAX || m_pipeNodesEditing.size() == 0) {
			return;
		}
		if(m_pipeInsertData.isNewCross) {
			SEditorPipeNode * p = m_pipeNodesEditing.back();
			for(int i = 0; i < 2; i++) {
				if(p->nodes[i] && p->nodes[i + 2] && p->nodes[i]->nodes[i + 2] && p->nodes[i + 2]->nodes[i]) {
					p->nodes[i]->nodes[i + 2] = p->nodes[i + 2];
					p->nodes[i + 2]->nodes[i] = p->nodes[i];
				}
			}
			DestroyPipeNode(p);
			m_pipeNodesEditing.pop_back();
		}
	}
	void GiveBackPipeNodesEditingToInsertMode() {
		DrawMyPipe(m_pipeNodesEditing);

		MovePipeNodesEditingToPipeNodes();

		m_pipeToolMode = EPipeToolMode::Insert;
		PipeToolCleanUp();
	}

	/* -------------------- 镜头与鼠标 -------------------- */

	void CameraControl();

	bool m_bMouseOnGUI = false;
	bool m_bMBLeftPressed = false;
	bool m_bMBLeftHolding = false;
	bool m_bMBLeftReleased = false;
	bool m_bMBRightPressed = false;

	void SetMouseLabel(Texts::EText text) {
		m_mouseLabelText = text;
	}
	Texts::EText m_mouseLabelText = Texts::EText::EMPTY;

	/* -------------------- 其它绘制函数 -------------------- */

	void DrawMyPipe(const std::vector<SEditorPipeNode *> & pipe, float alpha = 1.f) const;

	void DrawDevicePreview(IDeviceData::EType type, const Frame::Vec2 & pos, float alpha, int dirIndex, float scale) const {
		DrawDevicePreview(type, pos, alpha, dirIndex, scale, 0, false);
	}
	void DrawDevicePreview(IDeviceData::EType type, const Frame::Vec2 & pos, float alpha, int dirIndex, float scale, Frame::ColorRGB customColor, bool useCustomColor = true) const;

	void DrawOperationPrompt(const Frame::Vec2 & leftBottom);

	/* -------------------- 未归类 -------------------- */

	void ButtonEnd(const Frame::Vec2 & rightBottom);

	void SummonMachine();

};