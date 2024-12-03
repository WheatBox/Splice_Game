#pragma once

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include "Application.h"
#include "Utility.h"

class CDeviceComponent;

constexpr float CONNECTOR_LENGTH = 32.f;
constexpr float CONNECTOR_HALF_LENGTH = CONNECTOR_LENGTH / 2.f;
constexpr float PIPE_CROSS_SIZE = 32.f;

struct IDeviceData {
	IDeviceData() = default;
	virtual ~IDeviceData() = default;
	enum EType {
		Unset = 0,
		Cabin,
		Shell, // 从 Shell 开始，都是玩家在编辑器中可以主动放置的装置（不包含仅用作标识的 END）
		Engine,
		Propeller,
		JetPropeller,
		Joint,
		END
	} device = EType::Unset;
};

struct SDeviceDataMachinePartJoint : public IDeviceData {

	SDeviceDataMachinePartJoint() = default;
	virtual ~SDeviceDataMachinePartJoint() = default;

	void Initialize(CDeviceComponent * pPointComp, CDeviceComponent * pBehindComp, float rotationAdd, int dirIndexPointTo_pComp) {
		m_pPointMachinePartDeviceComponent = pPointComp;
		m_pBehindMachinePartDeviceComponent = pBehindComp;
		m_rotationAdd = rotationAdd;
		m_dirIndexPointTo_pAnotherMachinePartDeviceComponent = dirIndexPointTo_pComp;
	}

	// 会检查存储的 CDeviceComponent * 在 CDeviceComponent::s_workingDevices 是否还存在
	// 若存在，则正常返回，若不存在，则返回 nullptr
	CDeviceComponent * GetPointMachinePartDeviceComponent() const;

	// 会检查存储的 CDeviceComponent * 在 CDeviceComponent::s_workingDevices 是否还存在
	// 若存在，则正常返回，若不存在，则返回 nullptr
	CDeviceComponent * GetBehindMachinePartDeviceComponent() const;

	float GetRotationAdd() const {
		return m_rotationAdd;
	}

	int GetPointDirIndex() const {
		return m_dirIndexPointTo_pAnotherMachinePartDeviceComponent;
	}

protected:
	CDeviceComponent * m_pPointMachinePartDeviceComponent = nullptr;
	CDeviceComponent * m_pBehindMachinePartDeviceComponent = nullptr;
	float m_rotationAdd = 0.f;
	int m_dirIndexPointTo_pAnotherMachinePartDeviceComponent = 0;
};

struct SCabinDeviceData : public IDeviceData {
	SCabinDeviceData() { device = EType::Cabin; }
	virtual ~SCabinDeviceData() = default;
};

struct SShellDeviceData : public IDeviceData {
	SShellDeviceData() { device = EType::Shell; }
	virtual ~SShellDeviceData() = default;
};

struct SEngineDeviceData : public IDeviceData {
	SEngineDeviceData() { device = EType::Engine; }
	virtual ~SEngineDeviceData() = default;

	static constexpr float smokeMax = .03f;
	float smoking = 0.f;
};

struct SPropellerDeviceData : public IDeviceData {
	SPropellerDeviceData() { device = EType::Propeller; }
	virtual ~SPropellerDeviceData() = default;
};

struct SJetPropellerDeviceData : public IDeviceData {
	SJetPropellerDeviceData() {
		device = EType::JetPropeller;

		smokeRotation1 = Frame::DegToRad(static_cast<float>(rand() % 360));
		smokeRotation2 = Frame::DegToRad(static_cast<float>(rand() % 360));
	}
	virtual ~SJetPropellerDeviceData() = default;

	static constexpr float accumulationMax = 2.5f;
	static constexpr float accumulationShowingMax = 2.f;

	float accumulating = 0.f;
	float accumulatingShowing = 0.f;
	float accumulatingShowingPrev = 0.f;

	float smokeRotation1 = 0.f;
	float smokeRotation2 = 0.f;
};

struct SJointDeviceData : public SDeviceDataMachinePartJoint {
	SJointDeviceData() { device = EType::Joint; }
	virtual ~SJointDeviceData() = default;
};

struct SDeviceTreeNode {
	SDeviceTreeNode(IDeviceData::EType type) {
		switch(type) {
		case IDeviceData::EType::Cabin: pDeviceData = new SCabinDeviceData {}; break;
		case IDeviceData::EType::Shell: pDeviceData = new SShellDeviceData {}; break;
		case IDeviceData::EType::Engine: pDeviceData = new SEngineDeviceData {}; break;
		case IDeviceData::EType::Propeller: pDeviceData = new SPropellerDeviceData {}; break;
		case IDeviceData::EType::JetPropeller: pDeviceData = new SJetPropellerDeviceData {}; break;
		case IDeviceData::EType::Joint: pDeviceData = new SJointDeviceData {}; break;
		}
	}
	virtual ~SDeviceTreeNode() {
		delete pDeviceData;
	}
	//std::vector<SDeviceTreeNode *> nodes;
	SDeviceTreeNode * nodes[4] {};
	IDeviceData * pDeviceData = nullptr;
};

constexpr bool IsMachinePartJoint(IDeviceData::EType type) {
	switch(type) {
	case IDeviceData::Joint:
		return true;
	}
	return false;
}

// 该函数仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
static inline const Frame::Vec2 & GetDevicePixelSize(IDeviceData::EType device) {
	static const Frame::Vec2 results[] = {
		{ 0.f },
		{ 96.f }, // Cabin
		{ 96.f }, // Shell
		{ 96.f }, // Engine
		{ 96.f, 240.f }, // Propeller
		{ 184.f, 96.f }, // Jet Propeller
		{ 96.f }, // Joint
	};
	return device > IDeviceData::EType::Unset && device < IDeviceData::EType::END ? results[device] : results[0];
}

// 该函数仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
static inline Frame::Vec2 GetDeviceMeterSize(IDeviceData::EType device) {
	return PixelToMeterVec2(GetDevicePixelSize(device));
}

static inline Frame::Vec2 GetDeviceInterfaceBias(IDeviceData::EType device, int dirIndexOfDevice, int dirIndexOfInterface, float rotationAdd) {
	Frame::Vec2 res { 0.f };
	int dirIndex = dirIndexOfInterface - dirIndexOfDevice;
	if(dirIndex < 0) {
		dirIndex += 4;
	}

	switch(device) {
	case IDeviceData::EType::JetPropeller:
		if(dirIndex == 1 || dirIndex == 3) {
			res.x = -44.f;
		}
		break;
	}

	return res.GetRotated(-GetRadianByDirIndex(dirIndexOfDevice) + rotationAdd);
}

static inline bool IsDeviceHasPipeInterface(IDeviceData::EType type) {
	switch(type) {
	case IDeviceData::Engine:
	case IDeviceData::Propeller:
	case IDeviceData::JetPropeller:
		return true;
	}
	return false;
}

struct SColorSet {
	SColorSet()
		: color1 { 0xFFFFFF }
		, color2 { 0xFFFFFF }
		, connector { 0xFFFFFF }
		, pipe { 0xFFFFFF }
	{}
	SColorSet(Frame::ColorRGB _color1, Frame::ColorRGB _color2, Frame::ColorRGB _connector, Frame::ColorRGB _pipe)
		: color1 { _color1 }
		, color2 { _color2 }
		, connector { _connector }
		, pipe { _pipe }
	{}
	Frame::ColorRGB color1, color2, connector, pipe;
};