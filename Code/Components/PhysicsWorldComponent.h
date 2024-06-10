#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "CameraComponent.h"

#include <queue>

namespace std {
	class thread;
}

class CPhysicsWorldComponent final : public Frame::IEntityComponent {
public:

	// 同一时间，仅能存在一个 CPhysicsWorldComponent
	static CPhysicsWorldComponent * s_pPhysicsWorldComponent;

	static std::thread * s_pPhysicsThread;

	static std::queue<std::function<void ()>> s_physicalizeQueue;

	virtual void Initialize() override;
	virtual void OnShutDown() override;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override {
		return Frame::EntityEvent::EFlag::BeforeUpdate
			| Frame::EntityEvent::EFlag::Render;
	}
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CPhysicsWorldComponent> type) {
		type.SetGUID("{0372C9CF-412C-49A1-8EDE-E6E2BF53A39B}");
	}

	void SetEditorWorking(bool b) {
		m_bEditorWorking = b;
	}
	bool GetEditorWorking() const {
		return m_bEditorWorking;
	}

	CCameraComponent * GetCameraComponent() {
		return m_pCameraComponent;
	}

	void Step(float timeStep);

private:

	CCameraComponent * m_pCameraComponent = nullptr;
	
	Frame::CEntity * m_pSmokeEmitterEntity = nullptr;

	bool m_bEditorWorking = false;

};