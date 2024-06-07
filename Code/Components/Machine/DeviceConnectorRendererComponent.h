#pragma once

#include <FrameEntity/IEntityComponent.h>

#include <unordered_set>

class CEditorDeviceComponent;
class CDeviceComponent;

class CDeviceConnectorRendererComponent final : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CDeviceConnectorRendererComponent> type) {
		type.SetGUID("{73A803CD-F47A-4D56-9CBD-F2448165D115}");
	}

	void Initialize(const std::unordered_set<CDeviceComponent *> * const pComps);
	void Initialize(const std::unordered_set<CEditorDeviceComponent *> * const pComps);

	void SetWorking(bool b) {
		m_bWorking = b;
	}
	bool GetWorking() const {
		return m_bWorking;
	}

private:

	bool m_bWorking = true;

	const std::unordered_set<CEditorDeviceComponent *> * m_pEditorDeviceComps = nullptr;
	const std::unordered_set<CDeviceComponent *> * m_pDeviceComps = nullptr;

};