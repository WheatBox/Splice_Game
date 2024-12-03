#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameAsset/Sprite.h>
#include <FrameRender/Renderer.h>

#include <vector>
#include <functional>

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
		virtual ~SLayer() = default;

		void SetSprite(Frame::CStaticSprite * pStaticSprite) {
			m_pStaticSprite = pStaticSprite;
			m_pAnimatedSprite = nullptr;
			m_bAnimated = false;
			m_frameCount = 1;

			__Changed();
		}
		void SetSprite(Frame::CAnimatedSprite * pAnimatedSprite) {
			m_pStaticSprite = nullptr;
			m_pAnimatedSprite = pAnimatedSprite;
			m_bAnimated = true;
			m_frameCount = m_pAnimatedSprite->GetFrameCount();

			__Changed();
		}
		bool IsAnimated() const {
			return m_bAnimated;
		}
		Frame::CStaticSprite * GetStaticSprite() const {
			return m_pStaticSprite;
		}
		Frame::CAnimatedSprite * GetAnimatedSprite() const {
			return m_pAnimatedSprite;
		}

		const Frame::SSpriteImage * GetCurrentImage() const {
			if(m_bAnimated) {
				return m_pAnimatedSprite ? m_pAnimatedSprite->GetFrame(m_currentFrame) : nullptr;
			} else {
				return m_pStaticSprite ? m_pStaticSprite->GetImage() : nullptr;
			}
		}

		void SetFrame(int frame) {
			m_currentFrame = frame;
			__Changed();
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
			__Changed();
		}
		Frame::ColorRGB GetColor() const {
			return m_color;
		}

		void SetAlpha(float alpha) {
			m_alpha = alpha;
			__Changed();
		}
		float GetAlpha() const {
			return m_alpha;
		}

		void SetRotation(float angle) {
			m_rotation = angle;
			__Changed();
		}
		void SetRotationDegree(float angle) {
			SetRotation(Frame::DegToRad(angle));
		}
		float GetRotation() const {
			return m_rotation;
		}
		float GetRotationDegree() const {
			return Frame::RadToDeg(m_rotation);
		}

		void SetScale(const Frame::Vec2 & scale) {
			m_scale = scale;
			__Changed();
		}
		const Frame::Vec2 & GetScale() const {
			return m_scale;
		}

		void SetOffset(const Frame::Vec2 & offset) {
			m_offset = offset;
			__Changed();
		}
		const Frame::Vec2 & GetOffset() const {
			return m_offset;
		}

		int GetFrameCount() const {
			return m_frameCount;
		}

		void SetExtraFunction(const std::function<void (float)> & func) {
			m_extraFunc = func;
		}
		const std::function<void (float)> & GetExtraFunction() const {
			return m_extraFunc;
		}

	public:

		Frame::CStaticSprite * m_pStaticSprite = nullptr;
		Frame::CAnimatedSprite * m_pAnimatedSprite = nullptr;

		bool m_bAnimated = false;
		int m_currentFrame = 0;
		float m_frameInterval = .1f; // 秒 | seconds

		Frame::ColorRGB m_color { 0xFFFFFF };
		float m_alpha = 1.f;
		float m_rotation = 0.f;
		Frame::Vec2 m_scale { 1.f };
		Frame::Vec2 m_offset { 0.f };

		int m_frameCount = 0;
		float m_frameIntervalCounting = 0.f;

		//bool m_bExtraFunc = false;
		std::function<void (float)> m_extraFunc; // 参数：float frameTime

	public:
		void __Changed() {
			__bChanged = true;
		}
		bool __bChanged = true;
	};

	bool bUpdating = true;
	bool bRendering = false;

	// 请确保 layers 中的所有精灵都处在同一纹理页
	void GetRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const {
		buffersToPushBack.insert(buffersToPushBack.end(), m_insBuffers.begin(), m_insBuffers.end());
	}
	std::vector<SLayer> layers {};

private:
	float frameTime = 0.f;

	std::vector<Frame::CRenderer::SInstanceBuffer> m_insBuffers;
	
	void CheckOrUpdateInsBuffers();
public:
	static inline const Frame::STextureVertexBuffer & GetTextureVertexBufferForInstances() {
		static Frame::STextureVertexBuffer texVertBuf { 0.f, 1.f, 0xFFFFFF, 1.f };
		return texVertBuf;
	}

};