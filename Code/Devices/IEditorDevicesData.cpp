#include "IEditorDevicesData.h"

#include "IDevicesData.h"

#include <FrameCore/Log.h>

std::unordered_map<Frame::GUID, std::unique_ptr<IEditorDeviceData>> & GetEditorDeviceRegistry() {
	static std::unordered_map<Frame::GUID, std::unique_ptr<IEditorDeviceData>> registry {};
	return registry;
}

std::vector<Frame::GUID> & GetEditorDeviceOrder() {
	static std::vector<Frame::GUID> order {};
	return order;
}

const SEditorDeviceTypeConfig & GetEditorDeviceConfig(const Frame::GUID & guid) {
	static const SEditorDeviceTypeConfig errorConfig {};
	const auto & p = GetEditorDeviceData(guid);
	return p ? p->GetConfig() : errorConfig;
}

const std::unique_ptr<IEditorDeviceData> & GetEditorDeviceData(const Frame::GUID & guid) {
	static std::unique_ptr<IEditorDeviceData> errorRes = nullptr;
	const auto & registry = GetEditorDeviceRegistry();
	auto it = registry.find(guid);
	if(it == registry.end()) {
		Frame::Log::Log(Frame::Log::ELevel::Fatal, "Unknown editor device GUID %llX-%llX", guid.high, guid.low);
		return errorRes;
	}
	return it->second;
}

std::map<int, SDeviceInterfaceDef> IEditorDeviceData::GetInterfaceDefs() const {
	std::map<int, SDeviceInterfaceDef> map {};
	auto & deviceDefs = GetDeviceDefs();
	for(auto & deviceDef : deviceDefs) {
		const std::map<int, SDeviceInterfaceDef> & interfaceDefs = GetDeviceData(deviceDef.deviceGUID)->GetInterfaceDefs();
		map.insert(interfaceDefs.begin(), interfaceDefs.end());
	}
	return map;
}
