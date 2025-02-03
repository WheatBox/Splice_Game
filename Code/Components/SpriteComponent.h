#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameAsset/Sprite.h>
#include <FrameRender/Renderer.h>

#include <vector>
#include <functional>

#define InsBufferGroupT std::vector<Frame::CRenderer::SInstanceBuffer>

class CSpriteComponent : public Frame::IEntityComponent {
public:

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{0A6967F0-19EF-421D-84BB-249BEF7B3095}");
	}

	virtual void Initialize() override {
		CreateInsBufferGroup();
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

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

		SLayer & SetSprite(Frame::CStaticSprite * pStaticSprite) {
			m_pStaticSprite = pStaticSprite;
			m_pAnimatedSprite = nullptr;
			m_bAnimated = false;
			m_frameCount = 1;

			__Changed();
			return * this;
		}
		SLayer & SetSprite(Frame::CAnimatedSprite * pAnimatedSprite) {
			m_pStaticSprite = nullptr;
			m_pAnimatedSprite = pAnimatedSprite;
			m_bAnimated = true;
			m_frameCount = m_pAnimatedSprite->GetFrameCount();

			__Changed();
			return * this;
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

		SLayer & SetFrame(int frame) {
			m_currentFrame = frame;
			__Changed();
			return * this;
		}
		int GetFrame() const {
			return m_currentFrame;
		}

		SLayer & SetFrameInterval(float seconds) {
			m_frameInterval = seconds;
			return * this;
		}
		float GetFrameInterval() const {
			return m_frameInterval;
		}

		SLayer & SetColor(Frame::ColorRGB color) {
			m_color = color;
			__Changed();
			return * this;
		}
		Frame::ColorRGB GetColor() const {
			return m_color;
		}

		SLayer & SetAlpha(float alpha) {
			m_alpha = alpha;
			__Changed();
			return * this;
		}
		float GetAlpha() const {
			return m_alpha;
		}

		SLayer & SetRotation(float angle) {
			m_rotation = angle;
			__Changed();
			return * this;
		}
		SLayer & SetRotationDegree(float angle) {
			SetRotation(Frame::DegToRad(angle));
			return * this;
		}
		float GetRotation() const {
			return m_rotation;
		}
		float GetRotationDegree() const {
			return Frame::RadToDeg(m_rotation);
		}

		SLayer & SetScale(const Frame::Vec2 & scale) {
			m_scale = scale;
			__Changed();
			return * this;
		}
		const Frame::Vec2 & GetScale() const {
			return m_scale;
		}

		SLayer & SetOffset(const Frame::Vec2 & offset) {
			m_offset = offset;
			__Changed();
			return * this;
		}
		const Frame::Vec2 & GetOffset() const {
			return m_offset;
		}

		int GetFrameCount() const {
			return m_frameCount;
		}

		SLayer & SetExtraFunction(const std::function<void (float)> & func) {
			m_extraFunc = func;
			return * this;
		}
		const std::function<void (float)> & GetExtraFunction() const {
			return m_extraFunc;
		}

		SLayer & SetInsBufferGroup(unsigned int groupIndex) {
			m_groupIndex = groupIndex;
			__Changed();
			return * this;
		}
		unsigned int GetInsBufferGroup() const {
			return m_groupIndex;
		}

		void Animate(const float frameTime) {
			if(!m_bAnimated) {
				return;
			}
			m_frameIntervalCounting += frameTime;
			if(m_frameIntervalCounting >= m_frameInterval) {
				m_frameIntervalCounting -= m_frameInterval;

				m_currentFrame++;
				if(m_currentFrame >= m_frameCount) {
					m_currentFrame -= m_frameCount;
				}

				__Changed();
			}
		}

	private:

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

		unsigned int m_groupIndex = 0;

	public:
		void __Changed() {
			__bChanged = true;
		}
		bool __bChanged = true;

		unsigned int __indexInGroup = 0;
	};

	bool bUpdating = true;
	bool bRendering = false;

	// 请确保 layers 中的所有精灵都处在同一纹理页
	void GetAllRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const {
		for(const auto & group : m_insBufferGroups) {
			buffersToPushBack.insert(buffersToPushBack.end(), group.begin(), group.end());
		}
	}
	// 请确保 layers 中的所有精灵都处在同一纹理页
	void GetRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack, unsigned int index) const {
		if(m_insBufferGroups.size() <= index) {
			return;
		}
		buffersToPushBack.insert(buffersToPushBack.end(), m_insBufferGroups[index].begin(), m_insBufferGroups[index].end());
	}

	void AddLayer(const SLayer & layer, unsigned int groupIndex) {
		if(groupIndex >= m_insBufferGroups.size()) {
			m_insBufferGroups.resize(groupIndex + 1);
		}

		unsigned int indInGroup = 0;
		for(const auto & _layer : layers) {
			indInGroup += _layer.GetInsBufferGroup() == groupIndex;
		}
		
		layers.push_back(layer);
		auto & newlayer = layers.back();
		newlayer.SetInsBufferGroup(groupIndex);
		newlayer.__indexInGroup = indInGroup;

		m_insBufferGroups[groupIndex].push_back({});
	}
	void AddLayer(const SLayer & layer) {
		AddLayer(layer, 0);
	}

	std::vector<SLayer> & GetLayers() {
		return layers;
	}
	const std::vector<SLayer> & GetLayers() const {
		return layers;
	}

private:
	std::vector<SLayer> layers {};
	float frameTime = 0.f;

	std::vector<InsBufferGroupT> m_insBufferGroups;
	
public:
	InsBufferGroupT * CreateInsBufferGroup() {
		m_insBufferGroups.push_back({});
		return & m_insBufferGroups.back();
	}
	InsBufferGroupT * GetDefaultInsBufferGroup() {
		return & m_insBufferGroups[0];
	}
	const InsBufferGroupT * GetDefaultInsBufferGroup() const {
		return & m_insBufferGroups[0];
	}
	std::vector<InsBufferGroupT> & GetInsBufferGroups() {
		return m_insBufferGroups;
	}
	const std::vector<InsBufferGroupT> & GetInsBufferGroups() const {
		return m_insBufferGroups;
	}

	// 当 bUpdating 为 true 时，事件轮询中会去自动执行
	// 或者也可以在别处去手动调用
	void CheckOrUpdateInsBuffers();

	static inline const Frame::STextureVertexBuffer & GetTextureVertexBufferForInstances() {
		static Frame::STextureVertexBuffer texVertBuf { 0.f, 1.f, 0xFFFFFF, 1.f };
		return texVertBuf;
	}

private:
	Frame::Matrix33 m_insBuffersAfterTransform { Frame::Matrix33::CreateIdentity() };
public:
	void SetInsBuffersAfterTransform(const Frame::Matrix33 & trans) {
		m_insBuffersAfterTransform = trans;
	}

};

#undef InsBufferGroupT