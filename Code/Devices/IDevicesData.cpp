#include "IDevicesData.h"

#include <FrameCore/Log.h>

#include "../Components/Machine/MachinePartComponent.h"

std::unordered_map<Frame::GUID, std::unique_ptr<IDeviceData>> & GetDeviceRegistry() {
	static std::unordered_map<Frame::GUID, std::unique_ptr<IDeviceData>> registry {};
	return registry;
}

const SDeviceTypeConfig & GetDeviceConfig(const Frame::GUID & guid) {
	static const SDeviceTypeConfig errorConfig {};
	const auto & p = GetDeviceData(guid);
	return p ? p->GetConfig() : errorConfig;
}

const std::unique_ptr<IDeviceData> & GetDeviceData(const Frame::GUID & guid) {
	static std::unique_ptr<IDeviceData> errorRes = nullptr;
	const auto & registry = GetDeviceRegistry();
	auto it = registry.find(guid);
	if(it == registry.end()) {
		Frame::Log::Log(Frame::Log::ELevel::Fatal, "Unknown device GUID %llX-%llX", guid.high, guid.low);
		return errorRes;
	}
	return it->second;
}

Frame::Vec2 IDeviceData::GetPosition() const {
	if(m_pBelongingMachinePart) {
		if(CRigidbodyComponent * pRigidbody = m_pBelongingMachinePart->GetRigidbody()) {
			return pRigidbody->GetPosition() + m_positionRelative.GetRotated(pRigidbody->GetRotation());
		}
	}
	return m_positionRelative;
}

float IDeviceData::GetRotation() const {
	if(m_pBelongingMachinePart) {
		if(CRigidbodyComponent * pRigidbody = m_pBelongingMachinePart->GetRigidbody()) {
			return pRigidbody->GetRotation() + m_rotationRelative;
		}
	}
	return m_rotationRelative;
}