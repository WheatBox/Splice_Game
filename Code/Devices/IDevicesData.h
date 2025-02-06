#pragma once

#include "DevicesMisc.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include "../Components/RigidbodyComponent.h" // for SBox2dShape

#include <memory>

class CSpriteComponent;

struct SDeviceTypeConfig {
	Frame::GUID guid;
	Frame::Vec2 size = 96.f; // 该值仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
	bool isMachinePartJoint = false;
};

template<typename T>
struct SDeviceType {
	static SDeviceTypeConfig config;
};

struct IDeviceData {
	IDeviceData() = default;
	virtual ~IDeviceData() = default;

protected:
	virtual IDeviceData * New() const = 0;
public:
	std::shared_ptr<IDeviceData> NewShared() const { return std::shared_ptr<IDeviceData> { New() }; }
	std::unique_ptr<IDeviceData> NewUnique() const { return std::unique_ptr<IDeviceData> { New() }; }

	virtual const SDeviceTypeConfig & GetConfig() const = 0;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) = 0;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) = 0;
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
