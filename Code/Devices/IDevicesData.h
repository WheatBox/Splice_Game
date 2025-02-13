#pragma once

#include "Misc.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include "../Components/RigidbodyComponent.h" // for SBox2dShape
#include "../Components/SpriteComponent.h"

#include <memory>

#include <box2d/id.h>

class CMachinePartComponent;

struct SDeviceTypeConfig {
	Frame::GUID guid;
	Frame::Vec2 size = 96.f; // 该值仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
	bool isMachinePartJoint = false;

	float addPower = 0.f; // 给发动机之类的装置准备的
	float maxPower = 0.f;
};

template<typename T>
struct SDeviceType {
	static SDeviceTypeConfig config;
};

struct IDeviceData {
	IDeviceData() = default;
	virtual ~IDeviceData() = default;

	Frame::Vec2 m_positionRelative;
	float m_rotationRelative = 0.f;
	std::vector<b2ShapeId> m_shapes;
	CSprite m_sprite;

	void SetRelativePositionRotation(const Frame::Vec2 & posRela, float rotRela) {
		m_positionRelative = posRela;
		m_rotationRelative = rotRela;

		m_sprite.SetInsBuffersAfterTransform(Frame::Matrix33::CreateTranslation(m_positionRelative) * Frame::Matrix33::CreateRotationZ(m_rotationRelative));
	}

	// CSpriteComponent 中的组
	static constexpr int staticGroup = 0;
	static constexpr int dynamicGroup = 1;
	static constexpr int staticTopGroup = 2;

protected:
	virtual IDeviceData * New() const = 0;
public:
	std::shared_ptr<IDeviceData> NewShared() const { return std::shared_ptr<IDeviceData> { New() }; }
	std::unique_ptr<IDeviceData> NewUnique() const { return std::unique_ptr<IDeviceData> { New() }; }

	virtual const SDeviceTypeConfig & GetConfig() const = 0;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) = 0;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) = 0;

	struct SPreStepParams {
		Frame::Vec2 facingDir;
		Frame::Vec2 machinePartTargetMovingDir;
	};
	struct SStepParams {
		float timeStep;
		float power;
		CMachinePartComponent * pMachinePart;

		float machinePartRot;
		Frame::Vec2 machinePartPos;

		float DeviceRot(const IDeviceData * device) const {
			return machinePartRot + device->m_rotationRelative;
		}
		Frame::Vec2 DevicePos(const IDeviceData * device) const {
			return machinePartPos + device->m_positionRelative.GetRotated(machinePartRot);
		}
	};
	// 返回要申请的动力值占比，如果 config.maxPower 设为 0.f，那么该函数不会被执行
	virtual float PreStep(const SPreStepParams &) { return 1.f; }
	virtual void Step(const SStepParams &) {}
};

extern std::unordered_map<Frame::GUID, std::unique_ptr<IDeviceData>> & GetDeviceRegistry();

const SDeviceTypeConfig & GetDeviceConfig(const Frame::GUID & guid);
template<typename T>
static inline const SDeviceTypeConfig & GetDeviceConfig() {
	return SDeviceType<T>::config;
}

const std::unique_ptr<IDeviceData> & GetDeviceData(const Frame::GUID & guid);
template<typename T>
static inline const std::unique_ptr<IDeviceData> & GetDeviceData() {
	return GetDeviceData(GetDeviceConfig<T>().guid);
}

template<typename T>
struct __DeviceRegister {
	__DeviceRegister() {
		T::Register(SDeviceType<T>::config);
		GetDeviceRegistry().insert({ SDeviceType<T>::config.guid, std::make_unique<T>() });
	}
	virtual ~__DeviceRegister() = default;
};

#define REGISTER_DEVICE(DeviceType) \
	template<> SDeviceTypeConfig SDeviceType<DeviceType>::config {}; \
	__DeviceRegister<DeviceType> ___Register##DeviceType##__COUNTER__ {};
