#pragma once

#include <FrameMath/Vector2.h>

#include "../Texts.h"

namespace GUI {

	struct SGUIMouseData {
		bool bMouseOnGUI = false;

		Frame::Vec2 mousePos {};

		Texts::EText mouseLabel = Texts::EText::EMPTY;

		bool bMBLeftPressed = false;
		bool bMBLeftHolding = false;
		bool bMBLeftReleased = false;

		void Synch();
	};

	extern SGUIMouseData gGUIMouseData;

}