#pragma once

#include "DevicesMisc.h"

#include "DevicesData.h"

#include <FrameMath/Vector2.h>
#include <FrameUtility/GUID.h>

#include <memory>

class CColliderComponent;
class CSpriteComponent;

// 连接其它编辑器装置的 接口 的定义
// 如果都是一些比较常规的接口（直接的上下左右），可以使用 EasyMakeEditorDeviceInterfaceDefs() 进行创建
struct SEditorDeviceInterfaceDef {
	Frame::Vec2 offset;
	float direction = 0.f;
};

// 编辑器装置 转换为 装置 的定义
// 注意一个编辑器装置可能会转换为多个装置（例如 关节/Joint）
// 每一个 SEditorDeviceDeviceDef 对象都只表示一个装置
// 推荐使用 MakeEditorDeviceDeviceDef() 进行创建
struct SEditorDeviceDeviceDef {
	Frame::GUID deviceGUID;
	Frame::Vec2 offset;
	float rotation = 0.f;
	std::vector<size_t> interfaceDefIndices; // 存储对应在 SEditorDeviceTypeConfig::interfaceDefs 中的下标
};

struct SEditorDeviceTypeConfig {
	Frame::GUID guid;
	size_t orderIndex = SIZE_MAX; // 对应在 GetEditorDeviceOrder() 中的下标，会在使用 REGISTER_EDITOR_DEVICE 时处理，不要手动设置它！

	Frame::Vec2 size = 96.f; // 该值仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
	
	std::vector<SEditorDeviceInterfaceDef> interfaceDefs;
	std::vector<SEditorDeviceDeviceDef> deviceDefs;

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
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) = 0;
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

static inline std::vector<SEditorDeviceInterfaceDef> EasyMakeEditorDeviceInterfaceDefs(const SEditorDeviceTypeConfig & config, std::initializer_list<int> dirDegs_only1of_0_90_180_270) {
	std::vector<SEditorDeviceInterfaceDef> defs;
	for(const auto & dirDeg : dirDegs_only1of_0_90_180_270) {
		float xMulti = 0.f, yMulti = 0.f;
		switch(dirDeg) {
		case 0: xMulti = .5f; break;
		case 90: yMulti = -.5f; break;
		case 180: xMulti = -.5f; break;
		case 270: yMulti = .5f; break;
		}
		defs.push_back({ { config.size.x * xMulti, config.size.y * yMulti }, Frame::DegToRad(static_cast<float>(dirDeg)) });
	}
	return defs;
}

template<typename DeviceDataType>
static inline SEditorDeviceDeviceDef MakeEditorDeviceDeviceDef(const Frame::Vec2 & offset, float rotation, const std::vector<size_t> & interfaceDefIndices) {
	SEditorDeviceDeviceDef def;
	def.deviceGUID = GetDeviceConfig<DeviceDataType>().guid;
	def.offset = offset;
	def.rotation = rotation;
	def.interfaceDefIndices = interfaceDefIndices;
	return def;
}

template<typename DeviceDataType>
static inline SEditorDeviceDeviceDef MakeEditorDeviceDeviceDef(size_t _interfaceDefs_size) {
	std::vector<size_t> interfaceDefIndices;
	for(size_t i = 0; i < _interfaceDefs_size; i++) {
		interfaceDefIndices.push_back(i);
	}
	return MakeEditorDeviceDeviceDef<DeviceDataType>(0.f, 0.f, interfaceDefIndices);
}

#define REGISTER_EDITOR_DEVICE(EditorDeviceType) \
	template<> SEditorDeviceTypeConfig SEditorDeviceType<EditorDeviceType>::config {}; \
	__EditorDeviceRegister<EditorDeviceType> ___Register##EditorDeviceType##__COUNTER__ {};

struct SCabinEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{9A62A287-E14A-4DB6-A18D-16FA12F9A026}";
		config.pencilEnable = false;
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 0, 90, 180, 270 });
		config.deviceDefs = { MakeEditorDeviceDeviceDef<SCabinDeviceData>(config.interfaceDefs.size()) };
	}

	virtual IEditorDeviceData * New() const override { return new SCabinEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SCabinEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};

struct SShellEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{DAE8D332-9CB3-4F9C-B719-1E263CD7E7C2}";
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 0, 90, 180, 270 });
		config.deviceDefs = { MakeEditorDeviceDeviceDef<SShellDeviceData>(config.interfaceDefs.size()) };
	}

	virtual IEditorDeviceData * New() const override { return new SShellEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SShellEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};

struct SEngineEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{BBEC9D20-41D5-4BE6-9FAE-2C411284EA56}";
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 0, 90, 180, 270 });
		config.deviceDefs = { MakeEditorDeviceDeviceDef<SEngineDeviceData>(config.interfaceDefs.size()) };
	}

	virtual IEditorDeviceData * New() const override { return new SEngineEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SEngineEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};

struct SPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{41936E12-2CB8-42F2-BAC1-0B0787C4993F}";
		config.size = { 96.f, 240.f };
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 180 });
		config.deviceDefs = { MakeEditorDeviceDeviceDef<SPropellerDeviceData>(config.interfaceDefs.size()) };
	}

	virtual IEditorDeviceData * New() const override { return new SPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};

struct SJetPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{E32EEDA2-225A-44B9-8C09-34B3597FE888}";
		config.size = { 184.f, 96.f };
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 180, 90, 270 });
		config.interfaceDefs[1].offset += { -44.f, 0.f };
		config.interfaceDefs[2].offset += { -44.f, 0.f };
		config.deviceDefs = { MakeEditorDeviceDeviceDef<SJetPropellerDeviceData>(config.interfaceDefs.size()) };
	}

	virtual IEditorDeviceData * New() const override { return new SJetPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJetPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};

struct SJointEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{488C14A5-EE3F-4B75-9FDF-24A21CC03B66}";
		config.interfaceDefs = EasyMakeEditorDeviceInterfaceDefs(config, { 0, 180 });
		// TODO - config.deviceDefs;
		config.isMachinePartJoint = true;
	}

	virtual IEditorDeviceData * New() const override { return new SJointEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJointEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
};