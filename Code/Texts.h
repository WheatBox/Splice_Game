﻿#pragma once

#include <FrameCore/BasicTypes.h>
#include <FrameMath/Vector2.h>

#include <vector>

// TODO - 做成外部文件的形式

namespace Texts {

	enum class ELanguage {
		Chinese,
		English
	};

	extern std::vector<UnicodeString> gTexts;

	enum EText : size_t {
		EMPTY,

		OK,
		Cancel,
		Apply,
		Reset,

		EditorToolHand,
		EditorToolPencil,
		EditorToolEraser,
		EditorToolPipe,
		EditorToolSwatches,
		EditorToolController,

		EditorToolPipe_ModePencil,
		EditorToolPipe_ModeEraser,
		EditorToolPipe_ModeInsert,

		EditorOperationPrompt_Camera,
		EditorOperationPrompt_Hand,
		EditorOperationPrompt_Pencil,
		EditorOperationPrompt_Pipe,
		EditorOperationPrompt_PipePencil_Drawing,
		EditorOperationPrompt_Controller_Setting,

		EditorEnd,
	};
	
	const UnicodeString & GetText(EText text);

	void DrawTextLabel(EText text, const Frame::Vec2 & pos);

	void InitializeTexts(ELanguage language);

}