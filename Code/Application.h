#pragma once

#include <FrameCore/IApplication.h>
#include <FrameMath/Vector2.h>
#include <FrameEntity/Entity.h>

#include "box2dIncluded.h"

#include <list>

class CApplication final : public Frame::IApplication {

protected:
	
	virtual void Initialize(int argc, char ** argv) override;

	virtual void MainLoopPriority() override;
	virtual void MainLoopLast() override;

public:

	void RemoveEntityAtTheEndOfThisFrame(Frame::EntityId entityId) {
		m_entitiesWillBeRemovedAtTheEndOfThisFrame.push_back(entityId);
	}

private:
	std::list<Frame::EntityId> m_entitiesWillBeRemovedAtTheEndOfThisFrame;

};

#define __MeterToPixelRatio 32.f

static inline float MeterToPixel(float meter) {
	return meter * __MeterToPixelRatio;
}
static inline float PixelToMeter(float pixel) {
	static float ratio = 1.f / __MeterToPixelRatio;
	return pixel * ratio;
}

static inline Frame::Vec2 MeterToPixelVec2(const Frame::Vec2 & meter) {
	return meter * __MeterToPixelRatio;
}
static inline Frame::Vec2 PixelToMeterVec2(const Frame::Vec2 & pixel) {
	static float ratio = 1.f / __MeterToPixelRatio;
	return pixel * ratio;
}

#undef __MeterToPixelRatio

extern CApplication * gApplication;
extern b2World * gWorld;

static inline void RemoveEntityAtTheEndOfThisFrame(Frame::EntityId entityId) {
	if(gApplication) {
		gApplication->RemoveEntityAtTheEndOfThisFrame(entityId);
	}
}