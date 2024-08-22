#pragma once

#include "../../GUI/Page.h"
#include "../../Controller.h"

class CEditorControllerElement final : public GUI::IElement {
public:
	CEditorControllerElement() = delete;
	virtual ~CEditorControllerElement() = default;

	virtual void Step() override;

private:
	Controller::EElement m_element;
	bool m_bDragging = false;
};

class CEditorControllerPage final : public GUI::CDraggableResizablePage {
public:
	CEditorControllerPage() = delete;
	virtual ~CEditorControllerPage() = default;

	virtual void Step() override;

	Controller::SController & GetController() {
		return m_controller;
	}
private:
	Controller::SController m_controller {};
};