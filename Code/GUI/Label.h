#pragma once

#include "IElement.h"

#include <functional>

namespace GUI {

	class CLabel
		: public IElement
		, public STextManager
	{
	public:
		typedef std::function<void (CLabel *)> FuncStep;

		CLabel() = delete;
		CLabel(const Frame::Vec2 & pos, Texts::EText text)
			: CLabel { pos, text, Frame::ETextHAlign::Left, Frame::ETextVAlign::Top }
		{}
		CLabel(const Frame::Vec2 & pos, Texts::EText text, Frame::ETextHAlign halign, Frame::ETextVAlign valign)
			: m_pos { pos }
		{
			SetText(text);
			SetAlign(halign, valign);
		}
		virtual ~CLabel() = default;

		virtual Frame::Vec2 GetLeftTopRelative() const override { return m_pos; }
		virtual Frame::Vec2 GetRightBottomRelative() const override { return m_pos; }
		virtual void SetPosRelative(const Frame::Vec2 & posRelative) override { m_pos = posRelative; }
		virtual void SetSize(const Frame::Vec2 &) override {}

		virtual bool PointInMe(const Frame::Vec2 &) const {
			return false;
		}

		virtual void OnMouseOnMe(const Frame::Vec2 &) override {}
		
		virtual void Step() override {
			if(m_funcStep) {
				m_funcStep(this);
			}
		}

		virtual void Draw() override {
			DrawText(m_pos, m_color, 1.f);
		}

		void SetFuncStep(const FuncStep & func) {
			m_funcStep = func;
		}

		void SetColor(Frame::ColorRGB color) {
			m_color = color;
		}

	protected:
		Frame::Vec2 m_pos;
		FuncStep m_funcStep;
		Frame::ColorRGB m_color = __FRONT_TEXT_COLOR;
	};

}