#include "Controller.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "Assets.h"

namespace Controller {
	
	void DrawAABB(EElement element, const Frame::Vec2 & pos, Frame::ColorRGB rgb, float alpha) {

		Frame::Vec2 lt = pos, rb = pos;

#define __FORMULA(_EElement, _SXXXElement) \
case EElement::_EElement: { auto [_lt, _rb] = _SXXXElement::GetAABBRelative(); lt += Vec2Cast(_lt * Controller::gridCellSize); rb += Vec2Cast(_rb * Controller::gridCellSize); } break;

		switch(element) {
			__FORMULA(Button, SButtonElement);
		}

#undef __FORMULA

		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, rgb, alpha);

	}

	void DrawPreview(EElement element, const Frame::Vec2 & pos, float alpha, const Frame::Vec2 & scale) {
		float scaleMulti = 1.f;
		const Frame::SSpriteImage * pImage = nullptr;

#define __FORMULA(_EElement, _scaleMulti, _Assets_EGUIStaticSprite) \
case EElement::_EElement: \
	scaleMulti = _scaleMulti; \
	if(Frame::CStaticSprite * pSpr = Assets::GetStaticSprite(Assets::EGUIStaticSprite::_Assets_EGUIStaticSprite)) { \
		pImage = pSpr->GetImage(); \
	} \
	break;

		switch(element) {
			__FORMULA(Button, 1.f, Controller_button_free);
		}

#undef __FORMULA

		if(!pImage) {
			return;
		}

		Frame::gRenderer->DrawSpriteBlended(pImage, pos, 0xFFFFFF, alpha, scale * scaleMulti, 0.f);
	}

}
