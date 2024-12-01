#pragma once

#include <FrameCore/Globals.h>
#include <FrameInput/Input.h>

#include "EditorComponent.h"
#include "../SpriteComponent.h"
#include "../ColliderComponent.h"

class CEditorDeviceComponent final : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CEditorDeviceComponent> type) {
		type.SetGUID("{D798F1EC-5A5E-4EB0-9E8F-85A94E24BF93}");
	}

	// 若可放置于此处（无碰撞）（也就意味着初始化成功）则返回 true
	bool Initialize(IDeviceData::EType type, int dirIndex);

	virtual void OnShutDown() override;

	void GetAvailableInterfaces(std::vector<CEditorComponent::SInterface> * outToPushBack);

	IDeviceData::EType GetDeviceType() const {
		return m_deviceType;
	}

	bool ConnectWith(CEditorDeviceComponent * pEDComp, int dirIndex);

	void UpdateColor(const SColorSet & colorSet) {
		m_colorSet = colorSet;
		for(size_t i = 0, siz = m_colorUpdatesInSpriteLayers.size(); i < siz; i++) {
			if(const auto & p = m_colorUpdatesInSpriteLayers[i]; p) {
				m_pSpriteComponent->layers[i].SetColor(colorSet.* p);
			}
		}
	}

	void DrawConnectors();

	void SetAlpha(float alpha) {
		m_alpha = alpha;
		if(!m_pSpriteComponent) {
			return;
		}
		for(auto & layer : m_pSpriteComponent->layers) {
			layer.SetAlpha(alpha);
		}
	}
	float GetAlpha() const {
		return m_alpha;
	}

	int GetDirIndex() const {
		return m_directionIndex;
	}

	const SColorSet & GetColorSet() const {
		return m_colorSet;
	}

	void SetWorking(bool b) {
		m_bWorking = b;
		if(m_pSpriteComponent) {
			m_pSpriteComponent->working = b;
		}
	}
	
	CEditorDeviceComponent * m_neighbors[4] {};

private:
	bool m_bWorking = true;

	CSpriteComponent * m_pSpriteComponent = nullptr;
	CColliderComponent * m_pColliderComponent = nullptr;

	IDeviceData::EType m_deviceType = IDeviceData::EType::Unset;

	SColorSet m_colorSet;

	std::vector<Frame::ColorRGB SColorSet::*> m_colorUpdatesInSpriteLayers; // 具体作用，见 UpdateColor()

	int m_directionIndex = 0;

	float m_alpha = 1.f;

public:

	static void GetEditorDeviceColliders(CColliderComponent * outColliderComp, IDeviceData::EType type, int dirIndex);

};