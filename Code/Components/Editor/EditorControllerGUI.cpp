#include "EditorControllerGUI.h"

void CEditorControllerPage::Step() {
	CDraggableResizablePage::Step();
	if(IsResizing() || IsDragging()) {
		return;
	}


}
