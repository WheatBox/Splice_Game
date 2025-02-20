#pragma once

#include <FrameMath/ColorMath.h>
#include <FrameMath/Vector2.h>

constexpr float CONNECTOR_LENGTH = 16.f;
constexpr float CONNECTOR_HALF_LENGTH = CONNECTOR_LENGTH / 2.f;
constexpr float PIPE_CROSS_SIZE = 32.f;

struct SColorSet {
	SColorSet()
		: color1 { 0xFFFFFF }
		, color2 { 0xFFFFFF }
		, connector { 0xFFFFFF }
		, pipe { 0xFFFFFF }
	{}
	SColorSet(Frame::ColorRGB _color1, Frame::ColorRGB _color2, Frame::ColorRGB _connector, Frame::ColorRGB _pipe)
		: color1 { _color1 }
		, color2 { _color2 }
		, connector { _connector }
		, pipe { _pipe }
	{}
	Frame::ColorRGB color1, color2, connector, pipe;
};

// CSpriteComponent 中的组
enum EDeviceSpriteGroup : int {
	eDSG_Static = 0,
	eDSG_Dynamic,
	eDSG_StaticTop,
	eDSG_StaticBottom,
};

// 连接其它装置的 接口 的定义
// 如果都是一些比较常规的接口（直接的上下左右），可以使用 EasyMakeDeviceInterfaceDefs() 进行创建
struct SDeviceInterfaceDef {
	// int ID = -1; // 唯一标识符，需手动设定，任何 >= 0 的数字都行，一个编辑器装置内不能有两个一样的 ID 的接口
	//     ↑ 这个现在改成 map 的键了

	Frame::Vec2 offset;
	float direction;
};