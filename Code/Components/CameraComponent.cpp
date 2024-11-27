#include "CameraComponent.h"

#include "../Utility.h"

REGISTER_ENTITY_COMPONENT(CCameraComponent);

float CCameraComponent::s_wannaZoomVal = 0.f;

void CCameraComponent::Initialize() {
	Frame::gInput->pMouse->SetScrollCallback([this](double, double y) {
		s_wannaZoomVal = static_cast<float>(y);
		});
}

Frame::EntityEvent::Flags CCameraComponent::GetEventFlags() const {
	return Frame::EntityEvent::EFlag::BeforeUpdate;
}

void CCameraComponent::ProcessEvent(const Frame::EntityEvent::SEvent & event) {
	if(!m_bWorking) {
		return;
	}

	switch(event.flag) {
	case Frame::EntityEvent::BeforeUpdate:
		CameraControl(true);
		break;
	}
}

void CCameraComponent::CameraControl(bool bCanZoom) {
	const Frame::Vec2 mouseCurrPosInScene = GetMousePosInScene();
	// const Frame::Vec2 leftTopPos = Frame::gCamera->GetPos() - Frame::Vec2Cast(Frame::gCamera->GetViewSize()) * .5f / Frame::gCamera->GetZoom();

	if(m_func_IsTryingToMoveCamera()) {
		Frame::gCamera->SetPos(Frame::gCamera->GetPos() + m_mousePosBeforeBeginToMoveCamera - mouseCurrPosInScene);

		SetFollowing(false);
		SetFollowRotation(false);
	} else {
		m_mousePosBeforeBeginToMoveCamera = mouseCurrPosInScene;
	}

	const float mouseAngleRelativeToCamCenter = (Frame::gInput->pMouse->GetPosition() - Frame::Vec2Cast(Frame::gCamera->GetWindowSize()) * .5f).Radian();
	if(m_func_IsTryingToRotateCamera()) {
		Frame::gCamera->SetRotation(Frame::gCamera->GetRotation() + mouseAngleRelativeToCamCenter - m_mouseAngleBeforeBeginToRotateCamera);

		SetFollowRotation(false);
	}
	m_mouseAngleBeforeBeginToRotateCamera = mouseAngleRelativeToCamCenter;

	if(s_wannaZoomVal != 0.f) {
		if(bCanZoom) {
			Frame::gCamera->SetZoom(Frame::Clamp(Frame::gCamera->GetZoom() * (1.f + static_cast<float>(s_wannaZoomVal) * .15f), .1f, 4.f));
			Frame::gCamera->SetPos(Frame::gCamera->GetPos() + (mouseCurrPosInScene - GetMousePosInScene()));
		}

		s_wannaZoomVal = 0.f;
	}

	if(GetFollowing()) {
		if(Frame::CEntity * pEnt = GetEntityBeingFollowed()) {
			Frame::gCamera->SetPos(pEnt->GetPosition());
			if(GetFollowRotation()) {
				Frame::gCamera->SetViewRotationDegree(pEnt->GetRotation());
			}
		}
	}
}
