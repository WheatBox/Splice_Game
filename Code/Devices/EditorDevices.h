﻿#pragma once

#include "IEditorDevicesData.h"

struct SCabinEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{9A62A287-E14A-4DB6-A18D-16FA12F9A026}";
		config.pencilEnable = false;
	}

	virtual IEditorDeviceData * New() const override { return new SCabinEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SCabinEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};

struct SShellEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{DAE8D332-9CB3-4F9C-B719-1E263CD7E7C2}";
	}

	virtual IEditorDeviceData * New() const override { return new SShellEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SShellEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};

struct SEngineEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{BBEC9D20-41D5-4BE6-9FAE-2C411284EA56}";
	}

	virtual IEditorDeviceData * New() const override { return new SEngineEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SEngineEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};

struct SPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{41936E12-2CB8-42F2-BAC1-0B0787C4993F}";
		config.size = { 96.f, 240.f };
	}

	virtual IEditorDeviceData * New() const override { return new SPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & spritee, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};

struct SJetPropellerEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{E32EEDA2-225A-44B9-8C09-34B3597FE888}";
		config.size = { 184.f, 96.f };
	}

	virtual IEditorDeviceData * New() const override { return new SJetPropellerEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJetPropellerEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};

struct SJointEditorDeviceData : public IEditorDeviceData {
	static void Register(SEditorDeviceTypeConfig & config) {
		config.guid = "{488C14A5-EE3F-4B75-9FDF-24A21CC03B66}";
		config.isMachinePartJoint = true;
	}

	virtual IEditorDeviceData * New() const override { return new SJointEditorDeviceData {}; }
	virtual const SEditorDeviceTypeConfig & GetConfig() const override { return SEditorDeviceType<SJointEditorDeviceData>::config; }

	virtual void DrawPreview(const Frame::Vec2 & pos, const SColorSet & colorSet, float alpha, float scale, float rot) const override;
	virtual void InitCollider(CColliderComponent * outColliderComp, float rot) override;
	virtual void InitSprite(CSprite & sprite, std::vector<Frame::ColorRGB SColorSet::*> & outLayerColors) override;

	virtual const std::vector<SEditorDeviceDeviceDef> & GetDeviceDefs() const override;
};