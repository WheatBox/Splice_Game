#include "GUI.h"

#include <FrameCore/Globals.h>
#include <FrameCore/Camera.h>
#include <FrameRender/Renderer.h>

#include "../Assets.h"

namespace GUI {

	std::shared_ptr<CGUI> gCurrentGUI = nullptr;

	CGUI::CGUI() {
		// TODO - 就先暂时这么用着了
		m_pFont = new Frame::CFont { Assets::GetFontFilename(), 16.f };
	}

	void CGUI::Work() {
		const float camZoomBeforeGUI = Frame::gCamera->GetZoom();
		const Frame::Vec2 camPosBeforeGUI = Frame::gCamera->GetPos();
		const float camRotBeforeGUI = Frame::gCamera->GetRotation();
		Frame::CFont * pFontBeforeGUI = Frame::gRenderer->pTextRenderer->GetFont();

		Frame::gCamera->SetZoom(1.f);
		Frame::gCamera->SetPos(Frame::Vec2Cast(Frame::gCamera->GetViewSize()) * .5f);
		Frame::gCamera->SetRotation(0.f);
		if(m_pFont) {
			Frame::gRenderer->pTextRenderer->SetFont(m_pFont);
		}

		gGUIMouseData.Synch();

		Elements_SynchPosAdd(0.f);
		Elements_CheckAndCallOnMouseOnMe(gGUIMouseData.mousePos);
		Elements_Step();
		Elements_Draw(nullptr);

		if(gGUIMouseData.mouseLabel != Texts::EText::EMPTY) {
			Texts::DrawTextLabel(gGUIMouseData.mouseLabel, Frame::gInput->pMouse->GetPosition() + Frame::Vec2 { 16.f, 0.f });
		}

		Frame::gCamera->SetZoom(camZoomBeforeGUI);
		Frame::gCamera->SetPos(camPosBeforeGUI);
		Frame::gCamera->SetRotation(camRotBeforeGUI);
		Frame::gRenderer->pTextRenderer->SetFont(pFontBeforeGUI);
	}

}