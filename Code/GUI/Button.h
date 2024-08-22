#pragma once

#include <FrameMath/Math.h>

#include "IElement.h"
#include "../Assets.h"

#include <functional>

namespace GUI {

#define __DEFAULT_SIZE { 64.f, 32.f }

	class CButtonBase : public IElement {
	public:
		typedef std::function<void(void)> Func;
		typedef std::function<bool(void)> FuncCustomHighlightingCondition;

		CButtonBase()
			: CButtonBase { 0.f, Func {} }
		{}
		CButtonBase(const Frame::Vec2 & posRelative, const Func & func)
			: CButtonBase { posRelative, __DEFAULT_SIZE, func }
		{}
		CButtonBase(const Frame::Vec2 & posRelative, const Frame::Vec2 & size, const Func & func)
			: m_posRelative { posRelative }
		{
			SetSize(size);
			SetFunc(func);
		}
		virtual ~CButtonBase() = default;

		virtual Frame::Vec2 GetLeftTopRelative() const override {
			return m_posRelative - m_sizeHalf;
		}
		virtual Frame::Vec2 GetRightBottomRelative() const override {
			return m_posRelative + m_sizeHalf;
		}
		virtual void SetPosRelative(const Frame::Vec2 & posRelative) override {
			m_posRelative = posRelative;
		}
		virtual void SetSize(const Frame::Vec2 & size) override {
			m_sizeHalf = size * .5f;
		}

		void SetFunc(const Func & func) {
			m_func = func;
		}
		void SetCustomHighlightingCondition(const FuncCustomHighlightingCondition & func) {
			m_funcCustomHighlightingCondition = func;
		}

		virtual void OnMouseOnMe(const Frame::Vec2 & mousePos) override;
		virtual void Draw() override;

		virtual void DrawContent() {}

		enum class EHighlightStyle {
			Whiten,
			Outline
		};
		void SetHighlightStyle(EHighlightStyle highlightStyle) {
			m_highlightStyle = highlightStyle;
		}
		EHighlightStyle GetHighlightStyle() const {
			return m_highlightStyle;
		}

		void SetOutlineHighlightOutlineWidth(float width) {
			m_outlineHighlightOutlineWidth = width;
		}

	protected:
		Frame::Vec2 m_posRelative, m_sizeHalf;
		bool m_bMouseOnMe = false;
		EHighlightStyle m_highlightStyle = EHighlightStyle::Whiten;
		float m_outlineHighlightOutlineWidth = 2.f;
		Func m_func;
		FuncCustomHighlightingCondition m_funcCustomHighlightingCondition;
	};

	class CTextButton
		: public CButtonBase
		, public STextManager
	{
	public:
		CTextButton() = delete;
		CTextButton(const Func & func, Texts::EText text)
			: CTextButton { 0.f, func, text }
		{}
		CTextButton(const Frame::Vec2 & posRelative, const Func & func, Texts::EText text)
			: CTextButton { posRelative, __DEFAULT_SIZE, func, text }
		{}
		CTextButton(const Frame::Vec2 & posRelative, const Frame::Vec2 & size, const Func & func, Texts::EText text)
			: CButtonBase { posRelative, size, func }
		{
			SetText(text);
			SetAlign(Frame::ETextHAlign::Center, Frame::ETextVAlign::Middle);
		}
		virtual ~CTextButton() = default;

		virtual void DrawContent() override {
			DrawText(m_posRelative);
		}
	};

	class CImageButton : public CButtonBase {
	public:
		CImageButton() = delete;
		CImageButton(const Func & func, Assets::EGUIStaticSprite eGUIStaticSpr, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CImageButton { 0.f, func, eGUIStaticSpr, _mouseLabel }
		{}
		CImageButton(const Frame::Vec2 & posRelative, const Func & func, Assets::EGUIStaticSprite eGUIStaticSpr, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CImageButton { posRelative, __DEFAULT_SIZE, func, eGUIStaticSpr, _mouseLabel }
		{}
		CImageButton(const Frame::Vec2 & posRelative, const Frame::Vec2 & size, const Func & func, Assets::EGUIStaticSprite eGUIStaticSpr, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CButtonBase { posRelative, size, func }
		{
			mouseLabel = _mouseLabel;
			SetSprite(eGUIStaticSpr);
		}
		virtual ~CImageButton() = default;

		virtual void DrawContent() override;

		void SetSprite(Assets::EGUIStaticSprite spr) {
			m_eGUIStaticSpr = spr;
		}
		Assets::EGUIStaticSprite GetSprite() const {
			return m_eGUIStaticSpr;
		}

	protected:
		Assets::EGUIStaticSprite m_eGUIStaticSpr;
	};

	class CCustomDrawingButton : public CButtonBase {
	public:
		typedef std::function<void (const Frame::Vec2 & buttonCenter)> FuncDraw;

		CCustomDrawingButton() = delete;
		CCustomDrawingButton(const Func & func, const FuncDraw & funcDraw, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CCustomDrawingButton { 0.f, func, funcDraw, _mouseLabel }
		{}
		CCustomDrawingButton(const Frame::Vec2 & posRelative, const Func & func, const FuncDraw & funcDraw, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CCustomDrawingButton { posRelative, __DEFAULT_SIZE, func, funcDraw, _mouseLabel }
		{}
		CCustomDrawingButton(const Frame::Vec2 & posRelative, const Frame::Vec2 & size, const Func & func, const FuncDraw & funcDraw, Texts::EText _mouseLabel = Texts::EText::EMPTY)
			: CButtonBase { posRelative, size, func }
			, m_funcDraw { funcDraw }
		{
			mouseLabel = _mouseLabel;
		}
		virtual ~CCustomDrawingButton() = default;

		virtual void DrawContent() override {
			m_funcDraw(m_posRelative);
		}

	protected:
		FuncDraw m_funcDraw;
	};

#undef __DEFAULT_SIZE

}