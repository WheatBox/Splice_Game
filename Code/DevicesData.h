#pragma once

#include "DevicesMisc.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include "Application.h"
#include "Utility.h"
#include "Components/RigidbodyComponent.h" // for SBox2dShape

#include <memory>
#include <box2d/types.h>

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

struct SCabinDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{84D13CEA-E81E-48FE-8DC1-A9C318DF5580}";
	}

	virtual IDeviceData * New() const override { return new SCabinDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SCabinDeviceData>::config; }

	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;
};

struct SShellDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{3D691E5C-F162-4111-8F39-2E1596C22660}";
	}

	virtual IDeviceData * New() const override { return new SShellDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SShellDeviceData>::config; }

	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;
};

struct SEngineDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{9281278C-076B-4E0A-BFCC-E31BCD987513}";
	}

	virtual IDeviceData * New() const override { return new SEngineDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SEngineDeviceData>::config; }

	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	static constexpr float smokeMax = .03f;
	float smoking = 0.f;
};

struct SPropellerDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{A6E42B77-5B16-42B6-8FF2-429723B1A1AF}";
	}

	virtual IDeviceData * New() const override { return new SPropellerDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SPropellerDeviceData>::config; }

	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;
};

struct SJetPropellerDeviceData : public IDeviceData {
	SJetPropellerDeviceData() {
		smokeRotation1 = Frame::DegToRad(static_cast<float>(rand() % 360));
		smokeRotation2 = Frame::DegToRad(static_cast<float>(rand() % 360));
	}

	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{C29FBB21-3339-4DE5-818F-BA3F9609CB95}";
	}

	virtual IDeviceData * New() const override { return new SJetPropellerDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SJetPropellerDeviceData>::config; }

	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	static constexpr float accumulationMax = 2.5f;
	static constexpr float accumulationShowingMax = 2.f;

	float accumulating = 0.f;
	float accumulatingShowing = 0.f;
	float accumulatingShowingPrev = 0.f;

	float smokeRotation1 = 0.f;
	float smokeRotation2 = 0.f;
};

//struct SJointDeviceData : public IDeviceDataMachinePartJoint {
//	SJointDeviceData() { device = EType::Joint; }
//	virtual ~SJointDeviceData() = default;
//
//	virtual void InitSprite(CSpriteComponent *, std::vector<Frame::ColorRGB SColorSet::*> &) override {}
//	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 &, float) override { return {}; }
//};
