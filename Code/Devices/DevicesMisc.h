#pragma once

#include <FrameMath/ColorMath.h>

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
