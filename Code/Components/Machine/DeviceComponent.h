#pragma once

#include <FrameInput/Input.h>

#include "../SpriteComponent.h"
#include "MachineComponent.h"

#include "../../DevicesData.h"

#include <unordered_set>

class b2Fixture;
struct SPipeNode;

class CDeviceComponent final : public Frame::IEntityComponent {
public:

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CDeviceComponent> type) {
		type.SetGUID("{EC3F8403-9ADC-4AA2-BCA9-527240A20CDC}");
	}

	void Initialize(Frame::CEntity * pMachineEntity, IDeviceData::EType deviceType, Frame::EKeyId keyId, int dirIndex, const SColorSet & colorSet);
	virtual void OnShutDown() override;

	std::unordered_set<SPipeNode *> & GetPipeNodes() {
		return m_pipeNodes;
	}

	void SetGroup(CMachineComponent::SGroup * pGroup) {
		m_pGroup = pGroup;
	}
	CMachineComponent::SGroup * GetGroup() const {
		return m_pGroup;
	}

	void WeldWith(CDeviceComponent * pDeviceComp, int dirIndex);

	void DrawConnectors() const;

	void Work();

	static std::vector<b2FixtureDef *> CreateFixtureDefs(IDeviceData::EType deviceType, const Frame::Vec2 & devicePos, float rotation);
	static void DestroyFixtureDefs(const std::vector<b2FixtureDef *> & defs);
	
private:

	Frame::Vec2 m_relativePosition {};
	float m_relativeRotation = 0.f;

	Frame::CEntity * m_pMachineEntity = nullptr;
	CMachineComponent::SGroup * m_pGroup = nullptr;

	b2Fixture * m_pFixture = nullptr;

	float m_frametime = 0.f;

	CSpriteComponent * m_pSpriteComponent = nullptr;
	
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

	void SetFixture(b2Fixture * pFixture) {
		m_pFixture = pFixture;
	}
	b2Fixture * GetFixture() const {
		return m_pFixture;
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

};