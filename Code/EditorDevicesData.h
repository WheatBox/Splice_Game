#pragma once

#include "DevicesMisc.h"

#include <FrameMath/Vector2.h>

#include <memory>

class CColliderComponent;
class CSpriteComponent;

struct SEditorDeviceInterfaceDef {
	Frame::Vec2 offset;
	float direction;
};

struct SEditorDeviceTypeConfig {
	size_t indexInRegistry = 0; // 会在使用 REGISTER_EDITOR_DEVICE 时处理，不要手动设置它！
	Frame::Vec2 size = 96.f; // 该值仅用于需要粗略知道装置大致尺寸的应用场景，例如编辑器中获取位于装置边缘处的接口位置
	std::vector<SEditorDeviceInterfaceDef> interfaceDefs;
	bool pencilEnable = true; // 是否显示在铅笔工具的列表中
	bool isMachinePartJoint = false;
};

template<typename T>
struct SEditorDeviceType {
	static SEditorDeviceTypeConfig config;
};

template<typename T>
static inline const SEditorDeviceTypeConfig & GetEditorDeviceConfig() {
	return SEditorDeviceType<T>::config;
}

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
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) = 0;
};

extern std::vector<std::unique_ptr<IEditorDeviceData>> & GetEditorDeviceRegistry();

template<typename T>
struct __EditorDeviceRegister {
	__EditorDeviceRegister() {
		T::Register(SEditorDeviceType<T>::config);
		SEditorDeviceType<T>::config.indexInRegistry = GetEditorDeviceRegistry().size();
		GetEditorDeviceRegistry().push_back(std::make_unique<T>());
	}
	virtual ~__EditorDeviceRegister() = default;
};

#define REGISTER_EDITOR_DEVICE(EditorDeviceType) \
	template<> SEditorDeviceTypeConfig SEditorDeviceType<EditorDeviceType>::config {}; \
	__EditorDeviceRegister<EditorDeviceType> ___Register##EditorDeviceType##__COUNTER__ {};

static inline std::vector<SEditorDeviceInterfaceDef> __EasyMakeInterfaceDef(const SEditorDeviceTypeConfig & config, std::initializer_list<int> dirDegs_only1of_0_90_180_270) {
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

struct SCabinEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.pencilEnable = false;
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 0, 90, 180, 270 });
	}

	virtual IEditorDeviceData * New() const override { return new SCabinEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SCabinEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};

struct SShellEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 0, 90, 180, 270 });
	}

	virtual IEditorDeviceData * New() const override { return new SShellEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SShellEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};

struct SEngineEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 0, 90, 180, 270 });
	}

	virtual IEditorDeviceData * New() const override { return new SEngineEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SEngineEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};

struct SPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.size = { 96.f, 240.f };
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 180 });
	}

	virtual IEditorDeviceData * New() const override { return new SPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};

struct SJetPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.size = { 184.f, 96.f };
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 180, 90, 270 });
		config.interfaceDefs[1].offset += { -44.f, 0.f };
		config.interfaceDefs[2].offset += { -44.f, 0.f };
	}

	virtual IEditorDeviceData * New() const override { return new SJetPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJetPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};

struct SJointEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.interfaceDefs = __EasyMakeInterfaceDef(config, { 0, 180 });
		config.isMachinePartJoint = true;
	}

	virtual IEditorDeviceData * New() const override { return new SJointEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJointEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSpriteComponent * pSpriteComponent, std::vector<Frame::ColorRGB SColorSet::*> & layerColors) override;
};