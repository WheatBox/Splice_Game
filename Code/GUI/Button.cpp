#include "Button.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"

namespace GUI {

	void CButtonBase::OnMouseOnMe(const Frame::Vec2 &) {
		m_bMouseOnMe = true;
		if(gGUIMouseData.bMBLeftPressed && m_func) {
			m_func();
		}
	}

	void CButtonBase::Draw() {
		const Frame::Vec2 lt = GetLeftTopRelative();
		const Frame::Vec2 rb = GetRightBottomRelative();
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __FRONT_COLOR, 1.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __FRONT_EDGE_COLOR, 1.f, __FRONT_EDGE_WIDTH);

		DrawContent();

		if(m_funcCustomHighlightingCondition && m_funcCustomHighlightingCondition()) {
			switch(m_highlightStyle) {
			case EHighlightStyle::Whiten:
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, __FRONT_HIGHLIGHT_ALPHA);
				break;
			case EHighlightStyle::Outline:
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, 1.f, m_outlineHighlightOutlineWidth);
				break;
			}
		} else
		if(m_bMouseOnMe) {
			switch(m_highlightStyle) {
			case EHighlightStyle::Whiten:
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, __FRONT_HIGHLIGHT_ALPHA);
				break;
			case EHighlightStyle::Outline:
				Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, .5f, m_outlineHighlightOutlineWidth);
				break;
			}
		}
		m_bMouseOnMe = false;
	}

	void CImageButton::DrawContent() {
		Frame::CStaticSprite * pSpr = Assets::GetStaticSprite(m_eGUIStaticSpr);
		if(!pSpr) {
			return;
		}
		const Frame::SSpriteImage * pImage = pSpr->GetImage();
		if(!pImage) {
			return;
		}
		const Frame::Vec2 scale = (m_sizeHalf * 2.f) / Frame::Vec2Cast(pImage->GetSize());
		Frame::gRenderer->DrawSpriteBlended(pImage, m_posRelative, 0xFFFFFF, 1.f, scale, 0.f);
	}

}