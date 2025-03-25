#pragma once

#include <FrameEntity/IEntityComponent.h>
#include <FrameRender/Renderer.h>

#include "../../Application.h"
#include "../../Devices/Misc.h"

#include <unordered_set>
#include <memory>

struct IDeviceData;
class CRigidbodyComponent;

class CMachinePartComponent final : public Frame::IEntityComponent {
public:
	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{AB350096-7228-439B-A3DE-228BD7596F1C}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override { return Frame::EntityEvent::Nothing; }
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent &) override {}

	// 调用前需要先设定好 成员变量 devices，然后填入该函数的参数中
	// 之所以做成参数是为了提醒自己
	void Initialize(const std::unordered_set<std::shared_ptr<IDeviceData>> & _this_devices, const SColorSet & colorSet);

	CRigidbodyComponent * GetRigidbody() const {
		return m_pRigidbodyComponent;
	}
	
	void Step(float timeStep, const Frame::Vec2 & targetMovingDir);
	void RenderReady();
	void Render(EDeviceSpriteGroup eDSG_) const;
private:
	struct {
		float transformCamRot;
		Frame::Vec2 transformCamPos;
		unsigned int texId;
	} m_renderingStuffs;
public:

	std::unordered_set<std::shared_ptr<IDeviceData>> devices;
	bool isMainPart = false;

private:
	CRigidbodyComponent * m_pRigidbodyComponent = nullptr;

	std::vector<Frame::CRenderer::SInstanceBuffer> m_connectorInsBuffers;

	std::vector<Frame::CRenderer::SInstanceBuffer> m_staticBottomInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_staticInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_dynamicInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_staticTopInsBuffers;
	std::vector<Frame::CRenderer::SInstanceBuffer> m_dynamicTopInsBuffers;

	bool m_bInsBuffersInited = false;

	void __RegenerateStaticInsBuffers();
	void __RegenerateDynamicInsBuffers();
};