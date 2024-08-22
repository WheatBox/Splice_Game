#pragma once

#include <FrameCore/IApplication.h>
#include <FrameMath/Vector2.h>
#include <FrameEntity/Entity.h>

#include "box2dIncluded.h"

#include <list>

struct GLFWwindow;
struct GLFWcursor;

class CApplication final : public Frame::IApplication {

protected:
	
	virtual void Initialize() override;

	virtual void MainLoopPriority() override;
	virtual void MainLoopLast() override;

public:

	void RemoveEntityAtTheEndOfThisFrame(Frame::EntityId entityId) {
		m_entitiesWillBeRemovedAtTheEndOfThisFrame.push_back(entityId);
	}

	enum ECursor {
		eCursor_Unknown,
		eCursor_Arrow,
		eCursor_Ibeam,
		eCursor_Crosshair,
		eCursor_Hand,
		eCursor_ResizeEW,
		eCursor_ResizeNS,
		eCursor_ResizeNWSE,
		eCursor_ResizeNESW,
		eCursor_ResizeAll,
		eCursor_NotAllowed,
	};

	void SetCursor(ECursor shape);

private:
	std::list<Frame::EntityId> m_entitiesWillBeRemovedAtTheEndOfThisFrame;

	GLFWcursor * m_cursors[11] {};
	int m_cursorCount = static_cast<int>(sizeof(m_cursors) / sizeof(* m_cursors));
	ECursor m_cursorCurr = ECursor::eCursor_Unknown;
	ECursor m_cursorWill = ECursor::eCursor_Unknown;

	GLFWwindow * m_pSubWindow = nullptr;
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