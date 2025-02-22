#pragma once

#include "IDevicesData.h"

struct SCabinDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{84D13CEA-E81E-48FE-8DC1-A9C318DF5580}";
	}

	virtual IDeviceData * New() const override { return new SCabinDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SCabinDeviceData>::config; }

	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;
};

struct SShellDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{3D691E5C-F162-4111-8F39-2E1596C22660}";
	}

	virtual IDeviceData * New() const override { return new SShellDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SShellDeviceData>::config; }

	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;
	
	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;
};

struct SEngineDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{9281278C-076B-4E0A-BFCC-E31BCD987513}";
		config.addPower = 1.f;
	}

	virtual IDeviceData * New() const override { return new SEngineDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SEngineDeviceData>::config; }

	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;

	static constexpr float smokeMax = .03f;
	float smoking = 0.f;
};

struct SPropellerDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{A6E42B77-5B16-42B6-8FF2-429723B1A1AF}";
		config.maxPower = 4.f;
	}

	virtual IDeviceData * New() const override { return new SPropellerDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SPropellerDeviceData>::config; }

	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;

	virtual float PreStep(const SPreStepParams & params) override;
	virtual void Step(const SStepParams & params) override;
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

	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;
	virtual std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;

	static constexpr float accumulationMax = 2.5f;
	static constexpr float accumulationShowingMax = 2.f;

	float accumulating = 0.f;
	float accumulatingShowing = 0.f;
	float accumulatingShowingPrev = 0.f;

	float smokeRotation1 = 0.f;
	float smokeRotation2 = 0.f;
};

struct SJointRootDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{C42643C4-946C-4E3E-AC9F-678F4AAD8B1F}";
		config.isJoint = true;
		config.isJointRoot = true;
	}

	virtual IDeviceData * New() const override { return new SJointRootDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SJointRootDeviceData>::config; }

	void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet:: *> & outLayerColors) override;
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;

	virtual void InitJoint(const std::vector<std::shared_ptr<IDeviceData>> & devices) override;
};

struct SJointSecondDeviceData : public IDeviceData {
	static void Register(SDeviceTypeConfig & config) {
		config.guid = "{76EC29AD-0184-408E-B8B0-1A2EFCE020B4}";
		config.isJoint = true;
	}

	virtual IDeviceData * New() const override { return new SJointSecondDeviceData {}; }
	virtual const SDeviceTypeConfig & GetConfig() const override { return SDeviceType<SJointSecondDeviceData>::config; }

	void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet:: *> & outLayerColors) override;
	std::vector<std::pair<b2ShapeDef, SBox2dShape>> MakeShapeDefs(const Frame::Vec2 & devicePos, float rotation) override;

	virtual const std::map<int, SDeviceInterfaceDef> & GetInterfaceDefs() override;
};