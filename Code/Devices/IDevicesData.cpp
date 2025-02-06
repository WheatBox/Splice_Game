#include "IDevicesData.h"

#include <FrameCore/Log.h>

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