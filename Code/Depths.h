#pragma once

#define __DepthT constexpr int

namespace Depths {
	__DepthT PhysicsWorld = 0;

	__DepthT Editor = -999999;
	__DepthT EditorDevice = -1000 - 100;
	__DepthT EditorDeviceConnectorRenderer = EditorDevice + 1;

	__DepthT Device = -1000;
	__DepthT DeviceConnectorRenderer = Device + 1;
	__DepthT Machine = Device - 1; // 负责绘制管道
	__DepthT SmokeEmitter = Machine - 1; // 负责绘制烟雾
}

#undef __DepthT