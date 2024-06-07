#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameAsset/Sprite.h>

#include <vector>

class CSpriteComponent : public Frame::IEntityComponent {

public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CSpriteComponent> type) {
		type.SetGUID("{0A6967F0-19EF-421D-84BB-249BEF7B3095}");
	}

	struct SLayer {

		SLayer(Frame::CStaticSprite * pStaticSprite, Frame::ColorRGB color, float alpha) {
			SetSprite(pStaticSprite);
			SetColor(color);
			SetAlpha(alpha);
		}
		SLayer(Frame::CAnimatedSprite * pAnimatedSprite, Frame::ColorRGB color, float alpha, float frameIntervalSeconds) {
			SetSprite(pAnimatedSprite);
			SetColor(color);
			SetAlpha(alpha);
			SetFrameInterval(frameIntervalSeconds);
		}

		void SetSprite(Frame::CStaticSprite * pStaticSprite) {
			m_pStaticSprite = pStaticSprite;
			m_pAnimatedSprite = nullptr;
			m_bStatic = true;
			m_frameCount = 1;
		}
		void SetSprite(Frame::CAnimatedSprite * pAnimatedSprite) {
			m_pStaticSprite = nullptr;
			m_pAnimatedSprite = pAnimatedSprite;
			m_bStatic = false;
			m_frameCount = m_pAnimatedSprite->GetFrameCount();
		}
		bool IsStaticSprite() const {
			return m_bStatic;
		}
		Frame::CStaticSprite * GetStaticSprite() const {
			return m_pStaticSprite;
		}
		Frame::CAnimatedSprite * GetAnimatedSprite() const {
			return m_pAnimatedSprite;
		}

		void SetFrame(int frame) {
			m_currentFrame = frame;
		}
		int GetFrame() const {
			return m_currentFrame;
		}

		void SetFrameInterval(float seconds) {
			m_frameInterval = seconds;
		}
		float GetFrameInterval() const {
			return m_frameInterval;
		}

		void SetColor(Frame::ColorRGB color) {
			m_color = color;
		}
		Frame::ColorRGB GetColor() const {
			return m_color;
		}

		void SetAlpha(float alpha) {
			m_alpha = alpha;
		}
		float GetAlpha() const {
			return m_alpha;
		}

		void SetRotation(float angle) {
			m_rotation = angle;
		}
		float GetRotation() const {
			return m_rotation;
		}

		void SetScale(const Frame::Vec2 & scale) {
			m_scale = scale;
		}
		const Frame::Vec2 & GetScale() const {
			return m_scale;
		}

		void SetOffset(const Frame::Vec2 & offset) {
			m_offset = offset;
		}
		const Frame::Vec2 & GetOffset() const {
			return m_offset;
		}

		int GetFrameCount() const {
			return m_frameCount;
		}

	public:

		Frame::CStaticSprite * m_pStaticSprite = nullptr;
		Frame::CAnimatedSprite * m_pAnimatedSprite = nullptr;

		bool m_bStatic = true;
		int m_currentFrame = 0;
		float m_frameInterval = .1f; // 秒 | seconds

		Frame::ColorRGB m_color { 0xFFFFFF };
		float m_alpha = 1.f;
		float m_rotation = 0.f;
		Frame::Vec2 m_scale { 1.f };
		Frame::Vec2 m_offset { 0.f };

		int m_frameCount = 0;
		float m_frameIntervalCounting = 0.f;
	};

	std::vector<SLayer> layers {};
	bool working = true;

};