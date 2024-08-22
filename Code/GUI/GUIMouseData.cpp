#include "GUIMouseData.h"

#include <FrameInput/Input.h>

#include "../Utility.h"

namespace GUI {

	SGUIMouseData gGUIMouseData {};

	void SGUIMouseData::Synch() {
		bMouseOnGUI = false;

		mousePos = GetMousePosInScene();

		mouseLabel = Texts::EText::EMPTY;

		auto mbLeftState = Frame::gInput->pMouse->GetInputState(Frame::eMBI_Left);
		bMBLeftPressed = mbLeftState & Frame::EInputState::eIS_Press;
		bMBLeftHolding = mbLeftState & Frame::EInputState::eIS_Hold;
		bMBLeftReleased = mbLeftState & Frame::EInputState::eIS_Release;
	}

}