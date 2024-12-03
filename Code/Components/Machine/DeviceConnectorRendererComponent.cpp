#include "DeviceConnectorRendererComponent.h"

#include <FrameEntity/Entity.h>

#include "DeviceComponent.h"
#include "../Editor/EditorDeviceComponent.h"
#include "../../Depths.h"

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

REGISTER_ENTITY_COMPONENT(CDeviceConnectorRendererComponent);

Frame::EntityEvent::Flags CDeviceConnectorRendererComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::Render;
}

void CDeviceConnectorRendererComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	if(!m_bWorking) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::Render:
	{
		if(m_pDeviceComps) {
			for(const auto pComp : * m_pDeviceComps) {
				pComp->DrawConnectors();
			}
		} else
		if(m_pEditorDeviceComps) {
			//for(const auto pComp : * m_pEditorDeviceComps) {
				// pComp->DrawConnectors();
						// TODO
			//}
		}
	}
	break;
	}
}

void CDeviceConnectorRendererComponent::Initialize(const std::unordered_set<CDeviceComponent *> * const pComps) {
	m_pDeviceComps = pComps;
	m_pEntity->SetZDepth(Depths::DeviceConnectorRenderer);
}
void CDeviceConnectorRendererComponent::Initialize(const std::unordered_set<CEditorDeviceComponent *> * const pComps) {
	m_pEditorDeviceComps = pComps;
	m_pEntity->SetZDepth(Depths::EditorDeviceConnectorRenderer);
}
