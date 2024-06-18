#include "DevicesData.h"

#include "Components/Machine/DeviceComponent.h"

CDeviceComponent * SDeviceDataMachinePartJoint::GetPointMachinePartDeviceComponent() const {
	if(m_pPointMachinePartDeviceComponent && CDeviceComponent::s_workingDevices.find(m_pPointMachinePartDeviceComponent) != CDeviceComponent::s_workingDevices.end()) {
		return m_pPointMachinePartDeviceComponent;
	}
	return nullptr;
}

CDeviceComponent * SDeviceDataMachinePartJoint::GetBehindMachinePartDeviceComponent() const {
	if(m_pBehindMachinePartDeviceComponent && CDeviceComponent::s_workingDevices.find(m_pBehindMachinePartDeviceComponent) != CDeviceComponent::s_workingDevices.end()) {
		return m_pBehindMachinePartDeviceComponent;
	}
	return nullptr;
}
