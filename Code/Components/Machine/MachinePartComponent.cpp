#include "MachinePartComponent.h"

#include <FrameCore/Globals.h>
#include <FrameCore/Camera.h>

#include "../../Devices/Devices.h"
#include "../RigidbodyComponent.h"
#include "../../Assets.h"

REGISTER_ENTITY_COMPONENT(CMachinePartComponent);

void CMachinePartComponent::Initialize(const std::unordered_set<std::shared_ptr<IDeviceData>> & _this_devices, const SColorSet & colorSet) {
	m_pRigidbodyComponent = m_pEntity->CreateComponent<CRigidbodyComponent>();

	const Frame::Vec2 entPos = PixelToMeterVec2(m_pEntity->GetPosition());

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { entPos.x, entPos.y };
	bodyDef.rotation = b2MakeRot(Frame::DegToRad(m_pEntity->GetRotation()));
	bodyDef.linearDamping = 1.f;
	bodyDef.angularDamping = 1.f;

	m_pRigidbodyComponent->Physicalize(bodyDef, _this_devices);
	m_pRigidbodyComponent->SetEnableRendering(true);
	m_pRigidbodyComponent->SetRenderingColor(Frame::ColorHSV { static_cast<uint16>(rand()) % 360u, 100, 100 }.ToRGB());
	m_pRigidbodyComponent->SetRenderingColorAlwaysLight(true);

	for(const auto & device : _this_devices) {
		if(!isMainPart && device->GetConfig().guid == GetDeviceConfig<SCabinDeviceData>().guid) {
			isMainPart = true;
		}
		
		device->m_pBelongingMachinePart = this;

		std::vector<Frame::ColorRGB SColorSet::*> colorUpdatesInSpriteLayers;
		device->InitSprite(device->m_sprite, colorUpdatesInSpriteLayers);
		for(size_t i = 0, siz = colorUpdatesInSpriteLayers.size(); i < siz; i++) {
			if(const auto & p = colorUpdatesInSpriteLayers[i]; p) {
				device->m_sprite.GetLayers()[i].SetColor(colorSet.* p);
			}
		}
	}
}

void CMachinePartComponent::Step(float timeStep, const Frame::Vec2 & targetMovingDir) {
	if(!m_pRigidbodyComponent) {
		return;
	}

	const float rot = m_pRigidbodyComponent->GetRotation();
	const Frame::Vec2 pos = m_pRigidbodyComponent->GetPosition();
	m_pEntity->SetRotation(rot);
	m_pEntity->SetPosition(pos);

	std::vector<std::pair<std::shared_ptr<IDeviceData>, float>> devicePowerRatios; // 用于存储所有将会消耗动力的装置
	float totalPower = 0.f;
	float totalRatio = 0.f;
	for(const auto & pDevice : devices) {
		totalPower += pDevice->GetConfig().addPower;
		if(pDevice->GetConfig().maxPower != 0.f) {
			IDeviceData::SPreStepParams params;
			params.machinePartTargetMovingDir = targetMovingDir;
			params.facingDir = Frame::Vec2 { 1.f, 0.f }.Rotate(rot + pDevice->m_rotationRelative);
			if(float powerRatio = pDevice->PreStep(params); powerRatio > 0.001f) {
				totalRatio += powerRatio;
				devicePowerRatios.push_back({ pDevice, powerRatio });
			}
		}
	}

	if(totalRatio == 0.f || totalPower == 0.f) {
		return;
	}

	float powerUsed = 0.f;
	for(auto & [pDevice, powerRatio] : devicePowerRatios) {
		IDeviceData::SStepParams params;
		params.timeStep = timeStep;
		params.power = Frame::Min((powerRatio / totalRatio) * totalPower, pDevice->GetConfig().maxPower);

		pDevice->Step(params);

		printf("%5.2f ", params.power);
		powerUsed += params.power;
	}
	printf("| (%.2f/%.2f)\n", powerUsed, totalPower);
}

void CMachinePartComponent::Render() {
	for(auto & pDevice : devices) {
		pDevice->m_sprite.CheckOrUpdateInsBuffers();
	}

	if(!m_bInsBuffersInited) {
		m_bInsBuffersInited = true;
		__RegenerateStaticInsBuffers();
	}
	__RegenerateDynamicInsBuffers();

	Frame::gCamera->PushOntoStack();

	{ // 统一的 位移、旋转 ，通过镜头变换来做到
		const float rot = Frame::gCamera->GetRotation() - m_pEntity->GetRotation();
		const Frame::Vec2 vEntCam = (Frame::gCamera->GetPos() - m_pEntity->GetPosition()).GetRotated(-Frame::gCamera->GetRotation());
		const Frame::Vec2 camCalib = vEntCam.GetRotated(rot) - vEntCam;
		Frame::gCamera->SetRotation(rot);
		Frame::gCamera->SetPos(vEntCam + camCalib);
	}

	const auto texId = Assets::GetStaticSprite(Assets::EDeviceStaticSprite::cabin)->GetImage()->GetTextureId();
	const Frame::STextureVertexBuffer & vertBuf = CSprite::GetTextureVertexBufferForInstances();
	Frame::gRenderer->DrawTexturesInstanced(texId, vertBuf, m_staticBottomInsBuffers);
	Frame::gRenderer->DrawTexturesInstanced(texId, vertBuf, m_staticInsBuffers);
	Frame::gRenderer->DrawTexturesInstanced(texId, vertBuf, m_dynamicInsBuffers);
	Frame::gRenderer->DrawTexturesInstanced(texId, vertBuf, m_staticTopInsBuffers);
	// TODO - 上面这些做成函数以供 Machine 统一调用，不然的话一个 MachinePart 的 static组 可能会覆盖另一个 MachinePart 的 staticTop组

	Frame::gCamera->PopFromStack();
}

void CMachinePartComponent::__RegenerateStaticInsBuffers() {
	m_staticBottomInsBuffers.clear();
	m_staticInsBuffers.clear();
	m_staticTopInsBuffers.clear();
	for(auto & pDevice : devices) {
		pDevice->m_sprite.GetRenderingInstanceData(m_staticBottomInsBuffers, eDSG_StaticBottom);
		pDevice->m_sprite.GetRenderingInstanceData(m_staticInsBuffers, eDSG_Static);
		pDevice->m_sprite.GetRenderingInstanceData(m_staticTopInsBuffers, eDSG_StaticTop);
	}
}

void CMachinePartComponent::__RegenerateDynamicInsBuffers() {
	m_dynamicInsBuffers.clear();
	for(auto & pDevice : devices) {
		pDevice->m_sprite.GetRenderingInstanceData(m_dynamicInsBuffers, eDSG_Dynamic);
	}
}