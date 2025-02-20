#pragma once

#include <FrameEntity/IEntityComponent.h>

#include "../../Devices/IDevicesData.h"

#include <unordered_set>
#include <mutex>

#include <nlohmann/json.hpp>

class CMachinePartComponent;

class CMachineComponent final : public Frame::IEntityComponent {
public:
	static std::unordered_set<CMachineComponent *> s_workingMachines;
	static std::mutex s_workingMachinesMutex;

	static void Register(Frame::SComponentTypeConfig & config) {
		config.SetGUID("{77F48F89-3F1F-4905-8783-6BDB8A759B71}");
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	void Initialize(const SColorSet & colorSet) {
		m_colorSet = colorSet;

		std::lock_guard<std::mutex> machinesLock { s_workingMachinesMutex };
		s_workingMachines.insert(this);
	}
	virtual void OnShutDown() override;

	void Step(float timeStep);

	const Frame::Vec2 & GetTargetMovingDir() const {
		return m_targetMovingDir;
	}

	std::string SerializeStr() const {
		return Serialize().dump();
	}
	void DeserializeStr(std::string_view str) {
		try {
			Deserialize(nlohmann::json::parse(str));
		} catch(const nlohmann::json::exception & e) {
			Frame::Log::Log(Frame::Log::ELevel::Error, "CMachineComponent::DeserializeStr(): Illegal JSON: %s", e.what());
		}
	}

	nlohmann::json Serialize() const;
	void Deserialize(const nlohmann::json & json);

private:

	SColorSet m_colorSet;

	std::unordered_set<CMachinePartComponent *> m_machineParts;

	// 该值的坐标系是建立于场景的，而非相对于物体本身的
	Frame::Vec2 m_targetMovingDir {};

	std::mutex m_stepMutex;

};

struct SSerializedDevice {
	Frame::GUID guid;
	Frame::Vec2 position; // 相对于 驾驶舱 Cabin 来说的
	float rotation;       // 同上

	struct SConnection {
		size_t to; // 连接的装置的下标
		int myInterfaceID; // 哪个接口连接着 to
		int toInterfaceID; // 接口连接到的 to 的接口，也就是 to 的什么接口连接着自己
	};
	std::vector<SConnection> connections;

	nlohmann::json ToJson() const {
		nlohmann::json currConnectionsJson = nlohmann::json::array();
		for(const auto & connection : connections) {
			currConnectionsJson.push_back({
				{ "to", connection.to },
				{ "ID", connection.myInterfaceID },
				{ "toID", connection.toInterfaceID }
				});
		}
		nlohmann::json currJson = {
			{ "guidhigh", guid.high },
			{ "guidlow", guid.low },
			{ "x", position.x },
			{ "y", position.y },
			{ "rot", rotation },
			{ "connections", currConnectionsJson }
		};
		return currJson;
	}

	void FromJson(const nlohmann::json & json) {
		guid.high = json["guidhigh"];
		guid.low = json["guidlow"];
		position.x = json["x"];
		position.y = json["y"];
		rotation = json["rot"];
		for(const auto & currConnectionsJson : json["connections"]) {
			SConnection connection;
			connection.to = currConnectionsJson["to"];
			connection.myInterfaceID = currConnectionsJson["ID"];
			connection.toInterfaceID = currConnectionsJson["toID"];
			connections.push_back(connection);
		}
	}

	static SSerializedDevice MakeFromJson(const nlohmann::json & json) {
		SSerializedDevice res;
		res.FromJson(json);
		return res;
	}
};

// 一个 SSerializedJointRelation 表示一处关节
struct SSerializedJointRelation {
	// 这个数组存储的下标指向其它装置，通过它们获取到的装置将会作为参数
	// 传递给 根装置（通过 SDeviceTypeConfig::isJointRoot 进行设定） 的 物理关节创建函数
	std::vector<size_t> deviceIndices;

	nlohmann::json ToJson() const {
		return deviceIndices;
	}

	void FromJson(const nlohmann::json & json) {
		deviceIndices = json.get<std::vector<size_t>>();
	}

	static SSerializedJointRelation MakeFromJson(const nlohmann::json & json) {
		SSerializedJointRelation res;
		res.FromJson(json);
		return res;
	}
};

struct SSerializedMachine {
	std::vector<SSerializedDevice> devices;
	std::vector<SSerializedJointRelation> jointRelations;

	nlohmann::json ToJson() const {
		nlohmann::json jsonDevices = nlohmann::json::array();
		nlohmann::json jsonJoints = nlohmann::json::array();
		for(const auto & device : devices) {
			jsonDevices.push_back(device.ToJson());
		}
		for(const auto & joint : jointRelations) {
			jsonJoints.push_back(joint.ToJson());
		}
		return {
			{ "devices", jsonDevices },
			{ "joints", jsonJoints }
		};
	}

	void FromJson(const nlohmann::json & json) {
		devices.clear();
		jointRelations.clear();

		const nlohmann::json jsonDevices = json["devices"];
		const nlohmann::json jsonJointRelations = json["joints"];

		for(const auto & deviceJson : jsonDevices) {
			devices.push_back(SSerializedDevice::MakeFromJson(deviceJson));
		}

		for(const auto & jointRelationJson : jsonJointRelations) {
			jointRelations.push_back(SSerializedJointRelation::MakeFromJson(jointRelationJson));
		}
	}

	static SSerializedMachine MakeFromJson(const nlohmann::json & json) {
		SSerializedMachine res;
		res.FromJson(json);
		return res;
	}
};