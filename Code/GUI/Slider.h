#pragma once

#include <FrameMath/ColorMath.h>

#include "IElement.h"

#include <functional>

namespace GUI {

	class CSlider : public IElement {
	public:
		typedef std::function<int ()> FuncSynchValue;
		typedef std::function<void (int value, int maxValue)> FuncOnValueChanged;

		CSlider() = delete;
		CSlider(const Frame::Vec2 & posRelative, float width, int maxValue, const FuncSynchValue & funcSynchValue, const FuncOnValueChanged & funcOnValueChanged)
			: m_posRelative { posRelative }
			, m_widthHalf { width * .5f }
			, m_maxValue { maxValue }
		{
			SetFuncSynchValue(funcSynchValue);
			SetFuncOnValueChanged(funcOnValueChanged);
		}
		virtual ~CSlider() = default;

		virtual Frame::Vec2 GetLeftTopRelative() const override {
			return { m_posRelative.x - m_widthHalf, m_posRelative.y - heightHalf };
		}
		virtual Frame::Vec2 GetRightBottomRelative() const override {
			return { m_posRelative.x + m_widthHalf, m_posRelative.y + heightHalf };
		}
		virtual void SetPosRelative(const Frame::Vec2 & posRelative) override {
			m_posRelative = posRelative;
		}
		virtual void SetSize(const Frame::Vec2 & size) override {
			m_widthHalf = size.x * .5f;
		}

		virtual void OnMouseOnMe(const Frame::Vec2 & /*mousePos*/) override {
			if(gGUIMouseData.bMBLeftPressed) {
				m_bDragging = true;
			}
			if(gGUIMouseData.bMBLeftReleased) {
				m_bDragging = false;
			}
		}

		virtual void Step() override;

		virtual void Draw() override;

		void SetMaxValue(int val) {
			m_maxValue = val;
		}
		void SetValue(int val) {
			m_value = val;
		}
		int GetMaxValue() const {
			return m_maxValue;
		}
		int GetValue() const {
			return m_value;
		}
		float GetValueNormalized() const {
			return static_cast<float>(m_value) / static_cast<float>(m_maxValue);
		}

		void SetFuncSynchValue(const FuncSynchValue & func) {
			m_funcSynchValue = func;
		}
		void SetFuncOnValueChanged(const FuncOnValueChanged & func) {
			m_funcOnValueChanged = func;
		}

		void SetBackgroundColor(Frame::ColorRGB color) {
			m_backgroundColor = color;
		}

	protected:
		static constexpr float height = 12.f;
		static constexpr float heightHalf = height * .5f;

		Frame::Vec2 m_posRelative;
		float m_widthHalf = 0.f;

		int m_maxValue = 5;
		int m_value = 0;

		bool m_bDragging = false;

		FuncSynchValue m_funcSynchValue;
		FuncOnValueChanged m_funcOnValueChanged;

		Frame::ColorRGB m_backgroundColor { 0x000000 };
	};

}