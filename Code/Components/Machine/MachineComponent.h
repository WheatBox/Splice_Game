#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "../../DevicesData.h"

#include <unordered_set>

class CEditorDeviceComponent;
class CMachinePartComponent;
struct SEditorPipeNode;

class CMachineComponent final : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CMachineComponent> type) {
		type.SetGUID("{77F48F89-3F1F-4905-8783-6BDB8A759B71}");
	}

	void Initialize(CEditorDeviceComponent * pDeviceCabin, const SColorSet & colorSet);
	virtual void OnShutDown() override;

private:

	std::unordered_set<CMachinePartComponent *> m_machineParts;

};
