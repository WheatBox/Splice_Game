#include "Page.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"
#include "../Application.h"

namespace GUI {

	void CPage::SetPosAdd(const Frame::Vec2 & _posAdd) {
		posAdd = _posAdd;
		Elements_SynchPosAdd(GetLeftTop());
	}

	void CPage::OnMouseOnMe(const Frame::Vec2 & mousePos) {
		Elements_CheckAndCallOnMouseOnMe(mousePos);
	}

	void CPage::Step() {
		Elements_Step();
	}

	void CPage::Draw() {
		const Frame::Vec2 lt = GetLeftTopRelative();
		const Frame::Vec2 rb = GetRightBottomRelative();
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __BACKGROUND_COLOR, 1.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, __BACKGROUND_EDGE_COLOR, 1.f, __BACKGROUND_EDGE_WIDTH);

		Elements_Draw(m_pFramebuffer);
		Frame::gRenderer->DrawSpriteBlended(m_pFramebuffer->GetImage(), lt, 0xFFFFFF, 1.f);
	}

	void CDraggablePage::OnMouseOnMe(const Frame::Vec2 & mousePos) {
		if(!m_bDragging) {
			Elements_CheckAndCallOnMouseOnMe(mousePos);
		}
		
		if(gGUIMouseData.bMBLeftPressed && !m_bDragging) {
			m_bDragging = true;
			m_relativeToMouse = GetLeftTop() - gGUIMouseData.mousePos;
		}
	}

	void CDraggablePage::Step() {
		if(gGUIMouseData.bMBLeftReleased) {
			m_bDragging = false;
		}

		if(m_bDragging) {
			SetPos(m_relativeToMouse + gGUIMouseData.mousePos);
		}

		const Frame::Vec2 menuMaxPos = Frame::Vec2Cast(Frame::gCamera->GetViewSize()) - m_size;

		Frame::Vec2 lt = GetLeftTop();

		if(lt.x > menuMaxPos.x) {
			lt.x = menuMaxPos.x;
		}
		if(lt.y > menuMaxPos.y) {
			lt.y = menuMaxPos.y;
		}

		if(lt.x < 0.f) {
			lt.x = 0.f;
		}
		if(lt.y < 0.f) {
			lt.y = 0.f;
		}

		SetPos(lt);

		Elements_Step();
	}

	void CDraggableResizablePage::OnMouseOnMe(const Frame::Vec2 & mousePos) {
		const Frame::Vec2 rb = GetRightBottom();
		const Frame::Vec2 halfSize { 8.f };
		if(Frame::PointInRectangle(mousePos, rb - halfSize, rb + halfSize)) {
			gApplication->SetCursor(CApplication::eCursor_ResizeNWSE);
			if(gGUIMouseData.bMBLeftPressed) {
				m_bResizing = true;
			}
			return;
		}
		if(!m_bResizing) {
			CDraggablePage::OnMouseOnMe(mousePos);
		}
	}

	void CDraggableResizablePage::Step() {
		if(gGUIMouseData.bMBLeftReleased) {
			m_bResizing = false;
		}

		if(!m_bResizing) {
			CDraggablePage::Step();
			return;
		}

		gApplication->SetCursor(CApplication::eCursor_ResizeNWSE);

		SetSize(gGUIMouseData.mousePos - GetLeftTop());
	}

	void CDraggableResizablePage::Draw() {
		CDraggablePage::Draw();
		const Frame::Vec2 rb = GetRightBottomRelative();
		Frame::gRenderer->pShapeRenderer->DrawLineBlended({ rb.x, rb.y - 12.f }, { rb.x - 12.f, rb.y }, 0xFFFFFF, 1.f, 2.f);
		Frame::gRenderer->pShapeRenderer->DrawLineBlended({ rb.x, rb.y - 6.f }, { rb.x - 6.f, rb.y }, 0xFFFFFF, 1.f, 2.f);
	}

}