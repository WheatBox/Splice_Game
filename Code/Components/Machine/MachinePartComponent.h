#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "../../DevicesData.h"
#include "../../Utility.h"
#include "FrameRender/Renderer.h"

#include <unordered_set>

class CEditorDeviceComponent;
class CDeviceComponent;
class CRigidbodyComponent;
class CMachineComponent;
struct SPipeNode;

// 该组件所属实体的坐标并不是视觉上所直观看到的坐标
// 实体的坐标与旋转从物理引擎中获取，具体请见该组件的 Update 事件
// 所以该组件所属实体的坐标初始状态下是 (0, 0)，旋转是 0
class CMachinePartComponent final : public Frame::IEntityComponent {
public:

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{5BC5522E-DABD-4EA4-9CD5-5136C89DF74B}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	void Initialize(std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> * out_map_EDCompDeviceComp_or_nullptr, CMachineComponent * pMachine, const std::unordered_set<CEditorDeviceComponent *> & editorDeviceComps, const SColorSet & colorSet);
	virtual void OnShutDown() override;

	typedef std::unordered_set<SPipeNode *> PipeT;

	struct SGroup {
		std::unordered_set<CDeviceComponent *> engines; // 仅有 Engine
		std::unordered_set<CDeviceComponent *> devices; // 不包含 Engine
		bool bEngineWorking = false;
		bool bEngineWorkingPrevFrame = false; // 只是用以防止当两个相同按键的 Engine 在同一组里时对于 bEngineWorking 控制的冲突

		void InsertAndBind(CDeviceComponent * pDeviceComp);
	};

	void OnDeviceShutDown(CDeviceComponent * pDeviceComp, const std::unordered_set<SPipeNode *> & pipeNodes, SGroup * pGroup);

	// 关于返回值的定义，见 CRigidbodyComponent::CreateJointWith()
	bool CreateJointWith(CMachinePartComponent * pAnotherMachinePartComp, CDeviceComponent * pJointComp);

	const std::unordered_set<CDeviceComponent *> & GetDeviceComponents() const {
		return m_deviceComponents;
	}

private:

	CMachineComponent * m_pMachineBelonging = nullptr;

	CRigidbodyComponent * m_pRigidbodyComponent = nullptr;

	Frame::CEntity * m_pDeviceConnectorRendererEntity = nullptr;

	SColorSet m_colorSet;

	std::unordered_set<CDeviceComponent *> m_deviceComponents;

	std::unordered_set<SGroup *> m_groups;

	std::vector<PipeT> m_pipes;

private:
	bool m_bInsBuffersInited = false;


	std::vector<Frame::CRenderer::SInstanceBuffer> m_staticInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_dynamicInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_staticTopInsBuffers;

	void __RegenerateStaticInsBuffers();
	void __RegenerateDynamicInsBuffers();

public:
	bool IsMainPart() const {
		return m_bMainPart;
	}

	const Frame::Vec2 & GetTargetMovingDir() const {
		return m_targetMovingDir;
	}
	void SetTargetMovingDir(const Frame::Vec2 & dir) {
		m_targetMovingDir = dir;
	}

	void Step(float timeStep);

private:
	bool m_bMainPart = false; // 主部分 就是 有驾驶舱的机器部分

	Frame::Vec2 m_targetMovingDir {};

};
