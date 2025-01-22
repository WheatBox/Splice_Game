#pragma once

#include <FrameInput/Input.h>

#include "../SpriteComponent.h"
#include "MachinePartComponent.h"
#include "../RigidbodyComponent.h"

#include "../../DevicesData.h"

#include <unordered_set>

struct SPipeNode;

class CDeviceComponent final : public Frame::IEntityComponent {
public:

	static std::unordered_set<CDeviceComponent *> s_workingDevices;

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CDeviceComponent> type) {
		type.SetGUID("{EC3F8403-9ADC-4AA2-BCA9-527240A20CDC}");
	}

	void Initialize(Frame::CEntity * pMachinePartEntity, IDeviceData::EType deviceType, Frame::EKeyId keyId, int dirIndex, const SColorSet & colorSet);
	virtual void OnShutDown() override;

	IDeviceData::EType GetDeviceType() const {
		if(m_pNode && m_pNode->pDeviceData) {
			return m_pNode->pDeviceData->device;
		}
		return IDeviceData::EType::Unset;
	}

	std::unordered_set<SPipeNode *> & GetPipeNodes() {
		return m_pipeNodes;
	}

	void SetGroup(CMachinePartComponent::SGroup * pGroup) {
		m_pGroup = pGroup;
	}
	CMachinePartComponent::SGroup * GetGroup() const {
		return m_pGroup;
	}

	void WeldWith(CDeviceComponent * pDeviceComp, int dirIndex);

	void DrawConnectors() const;

	void Step(float timeStep, float power, void * userdata = nullptr);

	static std::vector<std::pair<b2ShapeDef, CRigidbodyComponent::SBox2dShape>> MakeShapeDefs(IDeviceData::EType deviceType, const Frame::Vec2 & devicePos, float rotation);
	
private:

	Frame::Vec2 m_relativePosition {};
	float m_relativeRotation = 0.f;

	Frame::CEntity * m_pMachinePartEntity = nullptr;
	CMachinePartComponent::SGroup * m_pGroup = nullptr;

	float m_frametime = 0.f;

	CSpriteComponent * m_pSpriteComponent = nullptr;
	
	// VS 的鼠标悬停提示会显示类型为 PipeT，但是注意该变量的类型并不表示 PipeT
	// 也就是说存储的并非一条管道，而是直接附着于该装置上的管道节点
	std::unordered_set<SPipeNode *> m_pipeNodes;

	int m_directionIndex = 0;
	SDeviceTreeNode * m_pNode = nullptr;
	SColorSet m_colorSet;

	Frame::EKeyId m_keyId = Frame::EKeyId::eKI_Unknown;

public:

	void SetRelativePositionRotation(const Frame::Vec2 & pos, float rot) {
		m_relativePosition = pos;
		m_relativeRotation = rot;
	}
	float GetRelativeRotation() const {
		return m_relativeRotation;
	}

	Frame::CEntity * GetMachinePartEntity() const {
		return m_pMachinePartEntity;
	}

	int GetDirIndex() const {
		return m_directionIndex;
	}

	SDeviceTreeNode * GetNode() {
		return m_pNode;
	}

	const SColorSet & GetColorSet() const {
		return m_colorSet;
	}

public:

	static constexpr int staticInsBufferGroupIndex = 0;
	static constexpr int dynamicInsBufferGroupIndex = 1;
	static constexpr int staticTopInsBufferGroupIndex = 2;

	void GetRenderingInstanceData(std::vector<Frame::CRenderer::SInstanceBuffer> & buffersToPushBack, int insBufferGroupIndex) const {
		m_pSpriteComponent->GetRenderingInstanceData(buffersToPushBack, insBufferGroupIndex);
	}

};
