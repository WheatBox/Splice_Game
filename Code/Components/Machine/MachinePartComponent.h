#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "../../DevicesData.h"
#include "../../Utility.h"

#include <unordered_set>

class CEditorDeviceComponent;
class CDeviceComponent;
class CRigidbodyComponent;
class CMachineComponent;
struct SEditorPipeNode;
struct SPipeNode;

class CMachinePartComponent final : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CMachinePartComponent> type) {
		type.SetGUID("{5BC5522E-DABD-4EA4-9CD5-5136C89DF74B}");
	}

	void Initialize(std::unordered_map<CEditorDeviceComponent *, CDeviceComponent *> * out_map_EDCompDeviceComp_or_nullptr, CMachineComponent * pMachine, const std::unordered_set<CEditorDeviceComponent *> & editorDeviceComps, const std::unordered_set<const std::vector<SEditorPipeNode *> *> & pipes, const SColorSet & colorSet);
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

};