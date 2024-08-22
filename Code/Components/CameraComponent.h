#pragma once

#include <FrameCore/Globals.h>
#include <FrameEntity/EntitySystem.h>

#include <functional>

class CDeviceComponent;

class CCameraComponent final : public Frame::IEntityComponent {
public:

	virtual void Initialize() override;

	void Initialize(const std::function<bool()> & func_IsTryingToMoveCamera, const std::function<bool()> & func_IsTryingToRotateCamera) {
		m_func_IsTryingToMoveCamera = func_IsTryingToMoveCamera;
		m_func_IsTryingToRotateCamera = func_IsTryingToRotateCamera;
	}

	virtual Frame::EntityEvent::Flags GetEventFlags() const override;
	virtual void ProcessEvent(const Frame::EntityEvent::SEvent & event) override;

	static void Register(Frame::SComponentType<CCameraComponent> type) {
		type.SetGUID("{D9488C87-2807-41EE-8E78-85F59CDE2A05}");
	}

	void CameraControl(bool bCanZoom);

	// 暂停工作后，仍然可以通过在自己需要的位置上调用 CameraControl() 等函数来让该组件发挥作用
	// 也就是说，如果需要自己手动安排镜头控制和其它代码之间的执行顺序等，可以先将该组件设为暂停工作，然后在所需位置调用 CameraControl()
	void SetWorking(bool bWorking) {
		m_bWorking = bWorking;
	}
	bool GetWorking() const {
		return m_bWorking;
	}

	void SetFollowing(bool bFollowing) {
		m_bFollowingEntity = bFollowing;
	}
	bool GetFollowing() const {
		return m_bFollowingEntity;
	}

	// 当位置跟随也启用时才会跟着发挥作用，设置位置跟随的启用与否：SetFollowing()
	void SetFollowRotation(bool b) {
		m_bFollowRotation = b;
	}
	bool GetFollowRotation() const {
		return m_bFollowRotation;
	}

	void FollowEntity(Frame::CEntity * pEntity, bool bBeginToFollowIfNotFollowing) {
		if(!pEntity) {
			return;
		}
		m_entityIdToFollow = pEntity->GetId();
		if(bBeginToFollowIfNotFollowing) {
			SetFollowing(true);
		}
	}

	Frame::CEntity * GetEntityBeingFollowed() const {
		return Frame::gEntitySystem->GetEntity(m_entityIdToFollow);
	}

private:

	std::function<bool()> m_func_IsTryingToMoveCamera;
	std::function<bool()> m_func_IsTryingToRotateCamera;

	bool m_bWorking = false;

	bool m_bFollowingEntity = false;
	bool m_bFollowRotation = false;
	Frame::EntityId m_entityIdToFollow;

	static float s_wannaZoomVal;
	Frame::Vec2 m_mousePosBeforeBeginToMoveCamera {};
	float m_mouseAngleBeforeBeginToRotateCamera {}; // 此处的 "mouseAngle" 是指鼠标对于相机中心的角度

};