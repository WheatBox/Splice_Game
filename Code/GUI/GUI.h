#pragma once

#include "Page.h"
#include "Button.h"
#include "Slider.h"
#include "Label.h"

namespace Frame {
	class CFont;
}

namespace GUI {

	class CGUI final : public SContainerBase {
	public:
		CGUI();
		virtual ~CGUI() = default;

		void Work();

	private:
		Frame::CFont * m_pFont = nullptr;
	};

	extern std::shared_ptr<CGUI> gCurrentGUI;

	static inline void SetGUI(const std::shared_ptr<CGUI> & pGUI) {
		gCurrentGUI = pGUI;
	}
	static inline void UnsetGUI(const std::shared_ptr<CGUI> & pGUIToVerify = nullptr) {
		if(pGUIToVerify == nullptr || pGUIToVerify == gCurrentGUI) {
			gCurrentGUI = nullptr;
		}
	}
	static inline void SetOrUnsetGUI(const std::shared_ptr<CGUI> & pGUIToSetOrVerify, bool b_trueIsSet_falseIsUnset) {
		(b_trueIsSet_falseIsUnset ? SetGUI : UnsetGUI)(pGUIToSetOrVerify);
	}

}