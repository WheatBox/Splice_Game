#include "Texts.h"

#include <FrameUtility/UTF8Utils.h>
#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

namespace Texts {

	std::vector<UnicodeString> gTexts;

	const UnicodeString & GetText(EText text) {
		const static UnicodeString defaultText = Frame::UTF8Utils::ToUnicode("ERROR");
		return text < gTexts.size() ? gTexts[text] : defaultText;
	}

	void DrawTextLabel(EText text, const Frame::Vec2 & pos) {
		static const Frame::Vec2 edgeLT { -4.f, -2.f };
		static const Frame::Vec2 edgeRB { 4.f, 6.f };
		const auto & str = GetText(text);
		Frame::CFont * pFont = Frame::gRenderer->pTextRenderer->GetFont();
		const Frame::Vec2 siz = pFont->TextSize(str, 0.f);
		Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos + edgeLT, pos + edgeRB + siz, 0x000000, 1.f);
		Frame::gRenderer->pTextRenderer->DrawTextBlended(str, pos, 0xCCCCCC, 1.f);
	}

	void InitializeTexts(ELanguage language) {

		gTexts.clear();

#define __TEXT(chinese, english) gTexts.push_back(Frame::UTF8Utils::ToUnicode(language == ELanguage::Chinese ? chinese : english));

		__TEXT("", ""); // EMPTY

		__TEXT("确定", "OK"); // OK
		__TEXT("取消", "Cancel"); // Cancel
		__TEXT("应用", "Apply"); // Apply
		__TEXT("重置", "Reset"); // Reset

		__TEXT("[1] - 自由视角", "[1] - Free view"); // EditorToolHand
		__TEXT("[2] - 放置装置", "[2] - Place devices"); // EditorToolPencil
		__TEXT("[3] - 移除装置", "[3] - Remove devices"); // EditorToolEraser
		__TEXT("[4] - 编辑管道", "[4] - Edit pipes"); // EditorToolPipe
		__TEXT("[5] - 更改配色", "[5] - Change colors"); // EditorToolSwatches
		__TEXT("[6] - 编辑控制器", "[6] - Edit controller"); // EditorToolController

		__TEXT("放置管道", "Place pipe"); // EditorToolPipe_ModePencil
		__TEXT("移除管道", "Remove pipe"); // EditorToolPipe_ModeEraser
		__TEXT("分裂管道", "Split pipe"); // EditorToolPipe_ModeInsert

		__TEXT("[鼠标中键] - 移动视图，[鼠标滚轮] - 缩放视图，[1~6] - 切换工具", "[Mouse Middle Button] or [W/A/S/D] - Move view, [Move Scroll Wheel] - Zoom view, [1~6] - Switch tool"); // EditorOperationPrompt_Camera
		__TEXT("[鼠标左键] - 移动视图", "[Mouse Left Button] - Move view"); // EditorOperationPrompt_Hand
		__TEXT("[Q/E] - 上/下一个装置", "[Q/E] - Previous/Next device"); // EditorOperationPrompt_Pencil
		__TEXT("[Q/E] - 上/下一个模式", "[Q/E] - Previous/Next mode"); // EditorOperationPrompt_Pipe
		__TEXT("[鼠标左键] - 继续，[鼠标右键] - 撤销", "[Mouse Left Button] - Continue, [Mouse Right Button] - Undo"); // EditorOperationPrompt_PipePencil_Drawing
		__TEXT("[任意] - 绑定，[Esc] - 清空，[鼠标右键] - 取消", "[Any] - Bind, [Esc] - Clear, [Mouse Right Button] - Cancel"); // EditorOperationPrompt_Controller_Setting

		__TEXT("我们走吧 !!", "Here we go!!"); // EditorEnd

#undef __TEXT
	}

}