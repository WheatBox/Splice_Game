#include "Pipe.h"

#include "Assets.h"
#include "Utility.h"

#include <map>

void DrawPipeSingleLine(Frame::Vec2 p1, const Frame::Vec2 & p2, Frame::ColorRGB color, float alpha) {
	constexpr float spacingToCross = 12.f;

	Frame::Vec2 v = p2 - p1;
	const Frame::Vec2 dir = v.GetNormalized();
	v -= dir * spacingToCross * 2.f;
	const float len = v.Length();
	const float degree = v.Degree();

	p1 += dir * spacingToCross;

	DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_color)->GetImage(), p1 + v * .5f, color, alpha, 0.f, { len / 32.f, 1.f }, degree);
	DrawSpriteBlendedPro(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe)->GetImage(), p1 + v * .5f, 0xFFFFFF, alpha, 0.f, { len / 32.f, 1.f }, degree);

	const Frame::Vec2 stop { std::abs(v.x), std::abs(v.y) };
	if(std::isnan(stop.x) || std::isnan(v.y)) {
		return;
	}
	constexpr float jointSpacing = 64.f;
	int i = static_cast<int>(len / jointSpacing);
	//for(Frame::Vec2 add = dir * jointSpacing * .75f; !(add.x > stop.x || add.x < -stop.x || add.y > stop.y || add.y < -stop.y); add += dir * jointSpacing) {
	for(Frame::Vec2 add = dir * jointSpacing * .75f; i-- > 0; add += dir * jointSpacing) {
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_joint_color)->GetImage(), p1 + add, color, alpha, 1.f, degree);
		Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(Assets::EDeviceStaticSprite::pipe_joint)->GetImage(), p1 + add, 0xFFFFFF, alpha, 1.f, degree);
	}
}