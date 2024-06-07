#pragma once

#include "SpriteComponent.h"

class CTestComponent : public Frame::IEntityComponent {
public:
	virtual void Initialize() override;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CTestComponent> type) {
		type.SetGUID("{CB9417C3-1124-4A99-A738-599E7BC7F13B}");
	}

	CSpriteComponent * m_pSpriteComponent = nullptr;
};