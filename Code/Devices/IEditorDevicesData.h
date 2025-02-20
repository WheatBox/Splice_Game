#pragma once

#include "Misc.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include <memory>
#include <map>

class CColliderComponent;
class CSprite;

// 编辑器装置 转换为 装置 的定义
// 注意一个编辑器装置可能会转换为多个装置（例如 关节/Joint）
// 每一个 SEditorDeviceDeviceDef 对象都只表示一个装置
// 推荐使用 MakeEditorDeviceDeviceDef() 进行创建
struct SEditorDeviceDeviceDef {
	Frame::GUID deviceGUID;
	Frame::Vec2 offset;
	float rotation = 0.f;
	std::vector<int> interfaceIDs; // 存储对应在 SEditorDeviceTypeConfig::interfaceDefs 中的 ID
};

struct SEditorDeviceTypeConfig {
	Frame::GUID guid;
	size_t orderIndex = SIZE_MAX; // 对应在 GetEditorDeviceOrder() 中的下标，会在使用 REGISTER_EDITOR_DEVICE 时处理，不要手动设置它！

	Frame::Vec2 size = 96.f; // 该值仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
	
	bool pencilEnable = true; // 是否显示在铅笔工具的列表中
	bool isMachinePartJoint = false;
};

template<typename T>
struct SEditorDeviceType {
	static SEditorDeviceTypeConfig config;
};

struct IEditorDeviceData {
	IEditorDeviceData() = default;
	virtual ~IEditorDeviceData() = default;

protected:
	virtual IEditorDeviceData * New() const = 0;
public:
	std::shared_ptr<IEditorDeviceData> NewShared() const { return std::shared_ptr<IEditorDeviceData> { New() }; }
	std::unique_ptr<IEditorDeviceData> NewUnique() const { return std::unique_ptr<IEditorDeviceData> { New() }; }

	virtual const SEditorDeviceTypeConfig & GetConfig() const = 0;

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const = 0;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) = 0;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) = 0;
	
	std::map<int, SDeviceInterfaceDef> GetInterfaceDefs() const;
	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const = 0;
};

extern std::unordered_map<Frame::GUID, std::unique_ptr<IEditorDeviceData>> & GetEditorDeviceRegistry();
extern std::vector<Frame::GUID> & GetEditorDeviceOrder(); // 为铅笔工具等需要顺序显示编辑器装置的地方所准备的

const SEditorDeviceTypeConfig & GetEditorDeviceConfig(const Frame::GUID & guid);
template<typename T>
static inline const SEditorDeviceTypeConfig & GetEditorDeviceConfig() {
	return SEditorDeviceType<T>::config;
}

const std::unique_ptr<IEditorDeviceData> & GetEditorDeviceData(const Frame::GUID & guid);
template<typename T>
static inline const std::unique_ptr<IEditorDeviceData> & GetEditorDeviceData() {
	return GetEditorDeviceData(GetEditorDeviceConfig<T>().guid);
}

template<typename T>
struct __EditorDeviceRegister {
	__EditorDeviceRegister() {
		T::Register(SEditorDeviceType<T>::config);
		GetEditorDeviceRegistry().insert({ SEditorDeviceType<T>::config.guid, std::make_unique<T>() });
		SEditorDeviceType<T>::config.orderIndex = GetEditorDeviceOrder().size();
		GetEditorDeviceOrder().push_back(SEditorDeviceType<T>::config.guid);
	}
	virtual ~__EditorDeviceRegister() = default;
};

template<typename DeviceDataType>
static inline SEditorDeviceDeviceDef MakeEditorDeviceDeviceDef(const std::vector<int> & interfaceIDs, const Frame::Vec2 & offset, float rotation) {
	SEditorDeviceDeviceDef def;
	def.deviceGUID = GetDeviceConfig<DeviceDataType>().guid;
	def.interfaceIDs = interfaceIDs;
	def.offset = offset;
	def.rotation = rotation;
	return def;
}

template<typename DeviceDataType>
static inline SEditorDeviceDeviceDef MakeEditorDeviceDeviceDef(const std::vector<int> & interfaceIDs) {
	return MakeEditorDeviceDeviceDef<DeviceDataType>(interfaceIDs, 0.f, 0.f);
}

template<typename DeviceDataType>
static inline SEditorDeviceDeviceDef MakeEditorDeviceDeviceDef() {
	std::vector<int> interfaceIDs;
	const auto & interfaceDefs = GetDeviceData<DeviceDataType>()->GetInterfaceDefs();
	for(const auto & interfaceDef : interfaceDefs) {
		interfaceIDs.push_back(interfaceDef.first);
	}
	return MakeEditorDeviceDeviceDef<DeviceDataType>(interfaceIDs, 0.f, 0.f);
}

#define REGISTER_EDITOR_DEVICE(EditorDeviceType) \
	template<> SEditorDeviceTypeConfig SEditorDeviceType<EditorDeviceType>::config {}; \
	__EditorDeviceRegister<EditorDeviceType> ___Register##EditorDeviceType##__COUNTER__ {};
