﻿#pragma once

#include <FrameMath/Vector2.h>
#include <FrameCore/Globals.h>
#include <FrameCore/Camera.h>
#include <FrameInput/Input.h>
#include <FrameMath/ColorMath.h>

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
void DrawSpriteBlendedPro(const Frame::SSpriteImage * pSpriteImage, const Frame::Vec2 & vPos, const Frame::ColorRGB & rgb, float alpha, float angle, const Frame::Vec2 & vScale, float angleAfterScale);

constexpr float Lerp(float a, float b, float t) {
	return a + (b - a) * t;
}