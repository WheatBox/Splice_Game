#include "IElement.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

namespace GUI {

	void SContainerBase::Elements_SynchPosAdd(const Frame::Vec2 & posAdd) {
		for(const auto & pElem : m_elements) {
			if(pElem->GetShowing()) {
				pElem->SetPosAdd(posAdd);
			}
		}
	}

	void SContainerBase::Elements_CheckAndCallOnMouseOnMe(const Frame::Vec2 & mousePos) {
		auto itClicked = m_elements.rend();
		for(auto it = m_elements.rbegin(); it != m_elements.rend(); it++) {
			if(* it && (* it)->GetShowing() && (* it)->PointInMe(mousePos)) {
				itClicked = it;
				break;
			}
		}

		if(itClicked != m_elements.rend()) {
			gGUIMouseData.mouseLabel = (* itClicked)->mouseLabel;

			if(gGUIMouseData.bMBLeftPressed) {
				auto it = itClicked.base();
				it--;
				m_elements.push_back(std::move(* it));
				m_elements.erase(it);
				itClicked = m_elements.rbegin();
			}

			(* itClicked)->OnMouseOnMe(mousePos);

			gGUIMouseData.bMBLeftPressed = false;
			gGUIMouseData.bMBLeftHolding = false;
			//gGUIMouseData.bMBLeftReleased = false;

			gGUIMouseData.bMouseOnGUI = true;
		}
	}

	void SContainerBase::Elements_Step() {
		for(const auto & pElem : m_elements) {
			if(pElem->GetShowing()) {
				pElem->Step();
			}
		}
	}

	void SContainerBase::Elements_Draw(const std::shared_ptr<Frame::CFramebuffer> & pFramebuffer) const {
		if(pFramebuffer) {
			pFramebuffer->Bind();
		}
		for(const auto & pElem : m_elements) {
			if(pElem->GetShowing()) {
				pElem->Draw();
			}
		}
		if(pFramebuffer) {
			pFramebuffer->Unbind();
		}
	}

	void STextManager::DrawText(Frame::Vec2 pos, Frame::ColorRGB color, float alpha) const {
		Frame::gRenderer->pTextRenderer->DrawTextAlignBlended(GetTextString(), pos, m_halign, m_valign, color, alpha);
	}

}