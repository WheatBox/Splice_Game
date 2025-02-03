#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "../../DevicesData.h"

#include <unordered_set>
#include <mutex>

class CEditorDeviceComponent;
class CMachinePartComponent;
struct SEditorPipeNode;

class CMachineComponent final : public Frame::IEntityComponent {
public:
	static std::unordered_set<CMachineComponent *> s_workingMachines;
	static std::mutex s_workingMachinesMutex;

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{77F48F89-3F1F-4905-8783-6BDB8A759B71}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	void Initialize(CEditorDeviceComponent * pDeviceCabin, const SColorSet & colorSet);
	virtual void OnShutDown() override;

	void Step(float timeStep);

	const Frame::Vec2 & GetTargetMovingDir() const {
		return m_targetMovingDir;
	}

private:

	std::unordered_set<CMachinePartComponent *> m_machineParts;

	// 该值的坐标系是建立于场景的，而非相对于物体本身的
	Frame::Vec2 m_targetMovingDir {};

	std::mutex m_stepMutex;

};
