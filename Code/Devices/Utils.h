#pragma once

#include "Misc.h"
#include "../Components/SpriteComponent.h"
#include "../Application.h"
#include "../Assets.h"

#define __SPRITE_ALPHA 1.f

static inline CSprite::SLayer & AddSpriteLayer(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB SColorSet::* col) {
	sprite.AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA });
	outLayerColors.push_back(col);
	return sprite.GetLayers().back();
}
static inline CSprite::SLayer & AddSpriteLayer(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite) {
	return AddSpriteLayer(sprite, outLayerColors, _EDeviceStaticSprite, nullptr);
}
static inline CSprite::SLayer & AddSpriteLayer(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors, Assets::EDeviceStaticSprite _EDeviceStaticSprite, Frame::ColorRGB SColorSet::* col, int _CDeviceComponent_InsBufferGroupIndex) {
	sprite.AddLayer({ Assets::GetStaticSprite(_EDeviceStaticSprite), 0xFFFFFF, __SPRITE_ALPHA }, _CDeviceComponent_InsBufferGroupIndex);
	outLayerColors.push_back(col);
	return sprite.GetLayers().back();
}

#undef __SPRITE_ALPHA

#define ADD_SPRITE_LAYER_1(_EDeviceStaticSprite) AddSpriteLayer(sprite, outLayerColors, _EDeviceStaticSprite)
#define ADD_SPRITE_LAYER_2(_EDeviceStaticSprite, col) AddSpriteLayer(sprite, outLayerColors, _EDeviceStaticSprite, col)
#define ADD_SPRITE_LAYER_3(_EDeviceStaticSprite, col, _CDeviceComponent_InsBufferGroupIndex) AddSpriteLayer(sprite, outLayerColors, _EDeviceStaticSprite, col, _CDeviceComponent_InsBufferGroupIndex)

constexpr float P2M(float pixel) {
	return PixelToMeter(pixel);
}
static inline Frame::Vec2 P2M(const Frame::Vec2 & pixel) {
	return PixelToMeterVec2(pixel);
}

using E = Assets::EDeviceStaticSprite;
using C = SColorSet;

static inline std::map<int, SDeviceInterfaceDef> EasyMakeDeviceInterfaceDefs(const Frame::Vec2 & deviceSize, std::initializer_list<int> IDs, std::initializer_list<int> dirDegs_only1of_0_90_180_270) {
	std::map<int, SDeviceInterfaceDef> defs;
	auto itID = IDs.begin();
	for(const auto & dirDeg : dirDegs_only1of_0_90_180_270) {
		float xMulti = 0.f, yMulti = 0.f;
		switch(dirDeg) {
		case 0: xMulti = .5f; break;
		case 90: yMulti = -.5f; break;
		case 180: xMulti = -.5f; break;
		case 270: yMulti = .5f; break;
		}
		defs.insert({ * itID++, { { deviceSize.x * xMulti, deviceSize.y * yMulti }, Frame::DegToRad(static_cast<float>(dirDeg)) } });
	}
	return defs;
}