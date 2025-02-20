#pragma once

#include "Misc.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include "../Components/RigidbodyComponent.h" // for SBox2dShape
#include "../Components/SpriteComponent.h"

#include <memory>
#include <map>

#include <box2d/id.h>

class CMachinePartComponent;

struct SDeviceTypeConfig {
	Frame::GUID guid;

	float addPower = 0.f; // 给发动机之类的装置准备的
	float maxPower = 0.f;

	bool isJoint = false; // 关节
	bool isJointRoot = false; // 关节根装置（将会执行 物理关节创建函数 的装置）
};

template<typename T>
struct SDeviceType {
	static SDeviceTypeConfig config;
};

struct IDeviceData {
	IDeviceData() = default;
	virtual ~IDeviceData() = default;

	CMachinePartComponent * m_pBelongingMachinePart = nullptr;
	Frame::Vec2 m_positionRelative;
	float m_rotationRelative = 0.f;
	std::vector<b2ShapeId> m_shapes;
	CSprite m_sprite;
	
	struct SInterface {
		SInterface(std::shared_ptr<IDeviceData> pDevice, int _ID)
			: SInterface(pDevice, _ID, pDevice->GetInterfaceDefs().at(_ID))
		{}
		SInterface(std::shared_ptr<IDeviceData> pDevice, int _ID, const SDeviceInterfaceDef & def) {
			ID = _ID;
			from = pDevice;
			direction = def.direction - pDevice->GetRotation();
			pos = pDevice->GetPosition()
				+ def.offset.GetRotated(pDevice->GetRotation())
				+ Frame::Vec2 { CONNECTOR_LENGTH, 0.f }.GetRotated(-direction)
				;
		}

		int ID = -1; // 与编辑器装置的接口ID对应
		std::weak_ptr<IDeviceData> from; // 该接口属于哪个装置
		SInterface * to = nullptr; // 该接口连接到的接口
		Frame::Vec2 pos {};
		float direction = 0.f;
	};
	std::map<int, SInterface> m_interfaces;

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

	void SetRelativePositionRotation(const Frame::Vec2 & posRela, float rotRela) {
		m_positionRelative = posRela;
		m_rotationRelative = rotRela;

		m_sprite.SetInsBuffersAfterTransform(Frame::Matrix33::CreateTranslation(m_positionRelative) * Frame::Matrix33::CreateRotationZ(m_rotationRelative));
	}

	Frame::Vec2 GetPosition() const;
	float GetRotation() const;

protected:
	virtual IDeviceData * New() const = 0;
public:
	std::shared_ptr<IDeviceData> NewShared() const { return std::shared_ptr<IDeviceData> { New() }; }
	std::unique_ptr<IDeviceData> NewUnique() const { return std::unique_ptr<IDeviceData> { New() }; }

	virtual const SDeviceTypeConfig & GetConfig() const = 0;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) = 0;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) = 0;
	
	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() = 0;

	struct SPreStepParams {
		Frame::Vec2 facingDir;
		Frame::Vec2 machinePartTargetMovingDir;
	};
	struct SStepParams {
		float timeStep;
		float power;
	};
	// 返回要申请的动力值占比，如果 config.maxPower 设为 0.f，那么该函数不会被执行
	virtual float PreStep(const SPreStepParams &) { return 1.f; }
	virtual void Step(const SStepParams &) {}

	// 只有 this->GetConfig().isJointRoot 为 true 的装置的该函数才会被调用
	// 参数是包括自己在内的该关节的所有的装置
	virtual void InitJoint(const std::vector<std::shared_ptr<IDeviceData>> &) {}
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
