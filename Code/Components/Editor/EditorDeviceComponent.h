#pragma once

#include <FrameCore/Globals.h>
#include <FrameInput/Input.h>

#include "../SpriteComponent.h"
#include "../ColliderComponent.h"
#include "FrameRender/Renderer.h"
#include "../../Devices/EditorDevices.h"

class CEditorDeviceComponent final : public Frame::IEntityComponent {
public:

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{D798F1EC-5A5E-4EB0-9E8F-85A94E24BF93}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	// 若可放置于此处（无碰撞）（也就意味着初始化成功）则返回 true
	bool Initialize(const Frame::GUID & editorDeviceGUID, float rotation);

	virtual void OnShutDown() override;

	bool IsCabin() const {
		return m_pData && dynamic_cast<SCabinEditorDeviceData *>(m_pData.get());
	}

	const std::shared_ptr<IEditorDeviceData> & GetData() const {
		return m_pData;
	}

	bool IsMachinePartJoint() const {
		return m_pData && m_pData->GetConfig().isMachinePartJoint;
	}

	void UpdateColor(const SColorSet & colorSet) {
		m_colorSet = colorSet;
		for(size_t i = 0, siz = m_colorUpdatesInSpriteLayers.size(); i < siz; i++) {
			if(const auto & p = m_colorUpdatesInSpriteLayers[i]; p) {
				m_pSpriteComponent->GetLayers()[i].SetColor(colorSet.* p);
			}
		}
	}

	void GetRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const {
		m_pSpriteComponent->GetAllRenderingInstanceData(buffersToPushBack);
	}
	void GetConnectorsRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack) const;

	void SetAlpha(float alpha) {
		m_alpha = alpha;
		if(!m_pSpriteComponent) {
			return;
		}
		for(auto & layer : m_pSpriteComponent->GetLayers()) {
			layer.SetAlpha(alpha);
		}
	}
	float GetAlpha() const {
		return m_alpha;
	}

	const SColorSet & GetColorSet() const {
		return m_colorSet;
	}

	void SetWorking(bool b) {
		m_bWorking = b;
		if(m_pSpriteComponent) {
			m_pSpriteComponent->bRendering = m_pSpriteComponent->bUpdating = b;
		}
	}

	// 各参数的具体解释见 CEditorDeviceComponent::Initialize()
	struct SInterface {
		int ID = -1;
		CEditorDeviceComponent * from = nullptr; // 该接口属于哪个装置
		SInterface * to = nullptr; // 该接口连接到的接口
		Frame::Vec2 pos {};
		float direction = 0.f;
	};
	std::vector<SInterface> m_interfaces;

	void GetAvailableInterfaces(std::vector<SInterface *> * outToPushBack) {
		for(SInterface & interface : m_interfaces) {
			if(!interface.to) {
				outToPushBack->push_back(& interface);
			}
		}
	}

	static bool ConnectInterfaces(CEditorDeviceComponent * pEDComp1, int interface1ID, CEditorDeviceComponent * pEDComp2, int interface2ID) {
		SInterface * interface1 = nullptr, * interface2 = nullptr;
		for(auto & interface : pEDComp1->m_interfaces) {
			if(interface.ID == interface1ID) {
				interface1 = & interface;
				break;
			}
		}
		for(auto & interface : pEDComp2->m_interfaces) {
			if(interface.ID == interface2ID) {
				interface2 = & interface;
				break;
			}
		}
		return ConnectInterfaces(interface1, interface2);
	}
	static bool ConnectInterfaces(SInterface * interface1, SInterface * interface2) {
		if(!interface1 || !interface2) {
			return false;
		}
		if(interface1->to || interface2->to) {
			return false;
		}
		interface1->to = interface2;
		interface2->to = interface1;
		return true;
	}
	static void DisconnectInterfaces(SInterface * interface) {
		if(!interface || !interface->to || interface->to->to != interface) {
			return;
		}
		interface->to->to = nullptr;
		interface->to = nullptr;
	}

private:
	bool m_bWorking = true;

	CSpriteComponent * m_pSpriteComponent = nullptr;
	CColliderComponent * m_pColliderComponent = nullptr;

	std::shared_ptr<IEditorDeviceData> m_pData = nullptr;

	SColorSet m_colorSet;

	std::vector<Frame::ColorRGB SColorSet::*> m_colorUpdatesInSpriteLayers; // 具体作用，见 UpdateColor()

	float m_alpha = 1.f;

};
