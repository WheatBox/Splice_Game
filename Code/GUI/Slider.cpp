#include "Slider.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "../Utility.h"

namespace GUI {
	
	void CSlider::Step() {
		if(gGUIMouseData.bMBLeftReleased) {
			m_bDragging = false;
		}
		if(m_bDragging) {
			gGUIMouseData.bMouseOnGUI = true;
			auto valueBefore = m_value;
			m_value = Frame::Clamp(static_cast<int>(std::round((gGUIMouseData.mousePos.x - GetLeftTop().x) / (m_widthHalf * 2.f) * static_cast<float>(m_maxValue))), 0, m_maxValue);
			if(m_value != valueBefore && m_funcOnValueChanged) {
				m_funcOnValueChanged(m_value, m_maxValue);
			}
		} else {
			if(m_funcSynchValue) {
				m_value = Frame::Clamp(m_funcSynchValue(), 0, m_maxValue);
			}
		}
	}

	void CSlider::Draw() {
		const Frame::Vec2 lt = GetLeftTopRelative();
		const Frame::Vec2 rb = GetRightBottomRelative();
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, m_backgroundColor, 1.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(lt, rb, 0xFFFFFF, 1.f, 1.f);
		
		//const Frame::Vec2 handleHalfSize { 2.f, heightHalf };
		//const Frame::Vec2 handlePos { lt.x + static_cast<float>(m_value) / static_cast<float>(m_maxValue) * (m_widthHalf - handleHalfSize.x) * 2.f + handleHalfSize.x, lt.y + (rb.y - lt.y) * .5f };
		//Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(handlePos - handleHalfSize, handlePos + handleHalfSize, 0xFFFFFF, 1.f);

		/*const Frame::Vec2 handleHalfSize { 10.f };
		const Frame::Vec2 handlePos { lt.x + static_cast<float>(m_value) / static_cast<float>(m_maxValue) * m_widthHalf * 2.f, lt.y + (rb.y - lt.y) * .5f };
		Frame::gRenderer->pShapeRenderer->DrawQuadrilateralBlended(
			{ handlePos.x - handleHalfSize.x, handlePos.y },
			{ handlePos.x, handlePos.y - handleHalfSize.y },
			{ handlePos.x, handlePos.y + handleHalfSize.y },
			{ handlePos.x + handleHalfSize.x, handlePos.y },
			0xFFFFFF, 1.f
		);*/

		const Frame::Vec2 handleHalfSize { 1.f, 9.f };
		const Frame::Vec2 handlePos { lt.x + static_cast<float>(m_value) / static_cast<float>(m_maxValue) * m_widthHalf * 2.f, lt.y + (rb.y - lt.y) * .5f };
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(handlePos - handleHalfSize, handlePos + handleHalfSize, 0xFFFFFF, 1.f);
	}

}