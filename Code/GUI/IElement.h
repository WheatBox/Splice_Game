#pragma once

#include <FrameMath/Math.h>
#include <FrameRender/Framebuffer.h>
#include <FrameRender/TextRenderer.h> // For Frame::ETextHAlign & Frame::ETextVAlign

#include <list>
#include <memory>

#include "GUIMouseData.h"
#include "Style.h"

namespace Frame {
	class CFramebuffer;
}

namespace GUI {

	struct IElement {
		IElement() = default;
		virtual ~IElement() = default;

		void __SetShowing(bool b, const Frame::Vec2 & synchPosAdd) {
			posAdd = synchPosAdd;
			bShowing = b;
		}

		void SetShowing(bool b) {
			bShowing = b;
		}
		bool GetShowing() const {
			return bShowing;
		}

	protected:
		bool bShowing = true;

	public:
		Texts::EText mouseLabel = Texts::EText::EMPTY;
		Frame::Vec2 posAdd { 0.f };

		virtual Frame::Vec2 GetLeftTopRelative() const = 0;
		virtual Frame::Vec2 GetRightBottomRelative() const = 0;
		virtual void SetPosRelative(const Frame::Vec2 & pos) = 0;
		virtual void SetSize(const Frame::Vec2 & size) = 0;
		
		Frame::Vec2 GetLeftTop() const {
			return posAdd + GetLeftTopRelative();
		}
		Frame::Vec2 GetRightBottom() const {
			return posAdd + GetRightBottomRelative();
		}
		void SetPos(const Frame::Vec2 & pos) {
			SetPosRelative(pos - posAdd);
		}

		virtual bool PointInMe(const Frame::Vec2 & point) const {
			return Frame::PointInRectangle(point, GetLeftTop(), GetRightBottom());
		}

		virtual void SetPosAdd(const Frame::Vec2 & _posAdd) {
			posAdd = _posAdd;
		}

		virtual void OnMouseOnMe(const Frame::Vec2 & mousePos) = 0;

		virtual void Step() {}

		virtual void Draw() = 0;
	};

	struct SContainerBase {
		SContainerBase() = default;
		virtual ~SContainerBase() = default;

		template<typename T, typename ... _Types>
		std::shared_ptr<T> CreateElement(_Types && ... _args) {
			auto p = std::make_shared<T>(_args ...);
			m_elements.push_back(p);
			return p;
		}

		void RemoveElement(const IElement * pElement) {
			m_elements.erase(std::remove_if(m_elements.begin(), m_elements.end(), [pElement](const std::shared_ptr<IElement> & p) { return p.get() == pElement; }), m_elements.end());
		}
		void RemoveElement(const std::shared_ptr<IElement> & pElement) {
			RemoveElement(pElement.get());
		}

	protected:
		std::list<std::shared_ptr<IElement>> m_elements;

		void Elements_SynchPosAdd(const Frame::Vec2 & posAdd);

		void Elements_CheckAndCallOnMouseOnMe(const Frame::Vec2 & mousePos);

		void Elements_Step();

		void Elements_Draw(const std::shared_ptr<Frame::CFramebuffer> & pFramebuffer) const;
	};

	struct STextManager {

		void DrawText(Frame::Vec2 pos) const {
			DrawText(pos, __FRONT_TEXT_COLOR, 1.f);
		}
		void DrawText(Frame::Vec2 pos, Frame::ColorRGB color, float alpha) const;

		void SetText(Texts::EText text) {
			m_text = text;
			m_string.clear();
			m_bUseStringButText = false;
		}
		void SetText(UTF8StringView str) {
			m_string = Frame::UTF8Utils::ToUnicode(str);
			m_bUseStringButText = true;
		}
		void SetText(UnicodeStringView str) {
			m_string = str;
			m_bUseStringButText = true;
		}
		Texts::EText GetText() const {
			return m_bUseStringButText ? Texts::EText::EMPTY : m_text;
		}
		UnicodeStringView GetTextString() const {
			return m_bUseStringButText ? m_string : Texts::GetText(m_text);
		}

		void SetHAlign(Frame::ETextHAlign halign) {
			m_halign = halign;
		}
		void SetVAlign(Frame::ETextVAlign valign) {
			m_valign = valign;
		}
		void SetAlign(Frame::ETextHAlign halign, Frame::ETextVAlign valign) {
			m_halign = halign;
			m_valign = valign;
		}
		Frame::ETextHAlign GetHAlign() const {
			return m_halign;
		}
		Frame::ETextVAlign GetVAlign() const {
			return m_valign;
		}

	protected:
		Texts::EText m_text = Texts::EText::EMPTY;
		UnicodeString m_string {};
		bool m_bUseStringButText = false; // 使用 m_text（为 false 时）还是 m_string（为 true 时）
		Frame::ETextHAlign m_halign = Frame::ETextHAlign::Left;
		Frame::ETextVAlign m_valign = Frame::ETextVAlign::Top;
	};

}