#pragma once

#include <FrameMath/Vector2.h>
#include <FrameCore/Globals.h>
#include <FrameCore/Camera.h>
#include <FrameInput/Input.h>
#include <FrameMath/ColorMath.h>
#include <FrameMath/Math.h>

#include <unordered_set>

static inline Frame::Vec2 GetMousePosInScene() {
	return Frame::gInput->pMouse->GetPositionInScene();
}

static inline float PointDistance(const Frame::Vec2 & p1, const Frame::Vec2 & p2) {
	return (p2 - p1).Length();
}

static inline const Frame::Vec2 & GetDirPosAdd(int dirIndex) {
	static const Frame::Vec2 dirPosAdd[4] = { { 1.f, 0.f }, { 0.f, -1.f }, { -1.f, 0.f }, { 0.f, 1.f } }; // 0 Right, 1 Up, 2 Left, 3 Down
	return dirPosAdd[dirIndex];
}

constexpr int GetRevDirIndex(int dirIndex) {
	constexpr int arr[4] = { 2, 3, 0, 1 };
	return arr[dirIndex];
}
constexpr int GetLeftDirIndex(int dirIndex) {
	constexpr int arr[4] = { 1, 2, 3, 0 };
	return arr[dirIndex];
}
constexpr int GetRightDirIndex(int dirIndex) {
	constexpr int arr[4] = { 3, 0, 1, 2 };
	return arr[dirIndex];
}

static inline int GetDirIndex(const Frame::Vec2 & v) {
	int dirIndex = (v.x == 0.f && v.y == 0.f) ? 0 : static_cast<int>((Frame::Vec2 { v.x, -v.y }.Degree() + 45.f) / 90.f);
	if(dirIndex < 0) {
		dirIndex += 4;
	} else
	if(dirIndex >= 4) {
		dirIndex -= 4;
	}
	return dirIndex;
}

static inline int GetDirIndexByDegree(float degree) {
	int dirIndex = static_cast<int>(std::floor((degree + 45.f) / 90.f));
	while(dirIndex < 0) {
		dirIndex += 4;
	}
	while(dirIndex >= 4) {
		dirIndex -= 4;
	}
	return dirIndex;
}

constexpr float GetDegreeByDirIndex(int dirIndex) {
	constexpr float arr[4] = { 0.f, 90.f, 180.f, -90.f };
	return arr[dirIndex];
}

constexpr float GetRadianByDirIndex(int dirIndex) {
	return Frame::DegToRad(GetDegreeByDirIndex(dirIndex));
}

static inline Frame::Vec2 GetRectangleEdgePosByDirIndex(const Frame::Vec2 & rectSize, int rectDirIndex, int edgeDirIndex) {
	return GetDirPosAdd(edgeDirIndex) * (((rectDirIndex == edgeDirIndex || rectDirIndex == GetRevDirIndex(edgeDirIndex)) ? rectSize.x : rectSize.y) * .5f);
}

// TODO - 这玩意应该做进引擎里，成为 ColorRGB 的一个成员函数
static inline Frame::ColorRGB ColorMultiply(Frame::ColorRGB col1, Frame::ColorRGB col2) {
	return {
		static_cast<uint8>(static_cast<int>(col1.r) * static_cast<int>(col2.r) / 255),
		static_cast<uint8>(static_cast<int>(col1.g) * static_cast<int>(col2.g) / 255),
		static_cast<uint8>(static_cast<int>(col1.b) * static_cast<int>(col2.b) / 255)
	};
}

// TODO - 这玩意应该也做进引擎里
const UnicodeString & GetKeyName(Frame::EKeyId keyId);

// TODO - 做进引擎里
Frame::EKeyId GetAnyKeyPressed();

// 注意！！！该函数仅可用于测试用途！！因为是随手写来作为测试辅助用的，所以运行速度极慢！！！
void DrawBlockBackground();

namespace Frame {
	struct SSpriteImage;
}
// TODO - 进引擎
void DrawSpriteBlendedPro(const Frame::SSpriteImage * pSpriteImage, const Frame::Vec2 & vPos, const Frame::ColorRGB & rgb, float alpha, float rotation, const Frame::Vec2 & vScale, float rotationAfterScale);

constexpr float Lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

class CEditorDeviceComponent;
class CDeviceComponent;

// 递归获取在同一机器部分中的装置
void RecursiveMachinePartEditorDevices(std::unordered_set<CEditorDeviceComponent *> * outSet, CEditorDeviceComponent * pComp);

// 递归获取在同一机器部分中的装置
void RecursiveMachinePartEditorDevices(std::unordered_set<CEditorDeviceComponent *> * outSet, std::unordered_set<CEditorDeviceComponent *> * outJointSet, CEditorDeviceComponent * pComp, const std::unordered_set<CEditorDeviceComponent *> & ignore);

int GetMachinePartJointDevicePointDirIndex(CEditorDeviceComponent * pEDComp);

class CMenuDragger final {
public:

	bool Work(bool bDragStart, bool bDragOver, const Frame::Vec2 & mousePosInScene, const Frame::Vec2 & menuSize, const Frame::Vec2 & areaLT, const Frame::Vec2 & areaRB) {
		WorkPart1(mousePosInScene, menuSize, areaLT, areaRB);
		WorkPart2(bDragStart, bDragOver, mousePosInScene, menuSize);
		return m_bDragging;
	}

	void WorkPart1(const Frame::Vec2 & mousePosInScene, const Frame::Vec2 & menuSize, const Frame::Vec2 & areaLT, const Frame::Vec2 & areaRB) {
		if(m_bDragging) {
			m_leftTop = m_leftTopRelativeToMouse + mousePosInScene;
		}

		const Frame::Vec2 menuMaxPos = areaRB - menuSize;

		if(m_leftTop.x > menuMaxPos.x) {
			m_leftTop.x = menuMaxPos.x;
		}
		if(m_leftTop.y > menuMaxPos.y) {
			m_leftTop.y = menuMaxPos.y;
		}

		if(m_leftTop.x < areaLT.x) {
			m_leftTop.x = areaLT.x;
		}
		if(m_leftTop.y < areaLT.y) {
			m_leftTop.y = areaLT.y;
		}
	}

	void WorkPart2(bool bDragStart, bool bDragOver, const Frame::Vec2 & mousePosInScene, const Frame::Vec2 & menuSize) {
		if(bDragStart && !m_bDragging && Frame::PointInRectangle(mousePosInScene, m_leftTop, m_leftTop + menuSize)) {
			m_bDragging = true;
			m_leftTopRelativeToMouse = m_leftTop - mousePosInScene;
		}
		if(bDragOver && m_bDragging) {
			m_bDragging = false;
		}
	}

	const Frame::Vec2 & GetLeftTop() const {
		return m_leftTop;
	}
	bool IsDragging() const {
		return m_bDragging;
	}

private:
	Frame::Vec2 m_leftTop {};
	Frame::Vec2 m_leftTopRelativeToMouse {};
	bool m_bDragging = false;
};