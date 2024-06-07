#include "Assets.h"

namespace Assets {

	std::map<EGUIStaticSprite, Frame::CStaticSprite *> gGUIStaticSpriteMap {};
	std::map<EDeviceStaticSprite, Frame::CStaticSprite *> gDeviceStaticSpriteMap {};

	static void SetSpriteOffsetToCenter(Frame::ISprite * pSpr) {
		pSpr->SetOffset({ .5f * static_cast<float>(pSpr->GetWidth()), .5f * static_cast<float>(pSpr->GetHeight()) });
	}

	void LoadPermanentAssets() {
#define __S_GUI(_e, _filename) gGUIStaticSpriteMap[EGUIStaticSprite::_e] = new Frame::CStaticSprite { "Assets/Art/GUI/" _filename };

		__S_GUI(Editor_tool_hand, "Editor/tool_hand.png");
		__S_GUI(Editor_tool_pencil, "Editor/tool_pencil.png");
		__S_GUI(Editor_tool_eraser, "Editor/tool_eraser.png");
		__S_GUI(Editor_tool_pipe, "Editor/tool_pipe.png");
		__S_GUI(Editor_tool_swatches, "Editor/tool_swatches.png");
		__S_GUI(Editor_tool_controller, "Editor/tool_controller.png");
		__S_GUI(Editor_tool_pipe_mode_pencil, "Editor/tool_pipe_mode_pencil.png");
		__S_GUI(Editor_tool_pipe_mode_eraser, "Editor/tool_pipe_mode_eraser.png");
		__S_GUI(Editor_tool_pipe_mode_insert, "Editor/tool_pipe_mode_insert.png");

#define __S_Device(_e, _filename) gDeviceStaticSpriteMap[EDeviceStaticSprite::_e] = new Frame::CStaticSprite { "Assets/Art/Devices/" _filename };

		__S_Device(connector, "connector.png");
		__S_Device(pipe, "pipe.png");
		__S_Device(pipe_color, "pipe_color.png");
		__S_Device(pipe_joint, "pipe_joint.png");
		__S_Device(pipe_joint_color, "pipe_joint_color.png");
		__S_Device(pipe_bend, "pipe_bend.png");
		__S_Device(pipe_bend_color, "pipe_bend_color.png");
		__S_Device(pipe_junction, "pipe_junction.png");
		__S_Device(pipe_junction_color, "pipe_junction_color.png");
		__S_Device(pipe_cross, "pipe_cross.png");
		__S_Device(pipe_cross_color, "pipe_cross_color.png");
		__S_Device(pipe_interface, "pipe_interface.png");
		__S_Device(pipe_interface_color, "pipe_interface_color.png");
		__S_Device(cabin, "cabin.png");
		__S_Device(cabin_color1, "cabin_color1.png");
		__S_Device(cabin_color2, "cabin_color2.png");
		__S_Device(cabin_logo_background, "cabin_logo_background.png");
		__S_Device(shell, "shell.png");
		__S_Device(shell_color1, "shell_color1.png");
		__S_Device(shell_color2, "shell_color2.png");
		__S_Device(engine, "engine.png");
		__S_Device(engine_color1, "engine_color1.png");
		__S_Device(engine_color2, "engine_color2.png");
		__S_Device(propeller_motor, "propeller_motor.png");
		__S_Device(propeller_motor_color1, "propeller_motor_color1.png");
		__S_Device(propeller_motor_color2, "propeller_motor_color2.png");
		__S_Device(propeller_top, "propeller_top.png");
		__S_Device(propeller_top_color, "propeller_top_color.png");
		__S_Device(propeller_blade, "propeller_blade.png");
		__S_Device(propeller_blade_color, "propeller_blade_color.png");
		__S_Device(jet_propeller, "jet_propeller.png");
		__S_Device(jet_propeller_color1, "jet_propeller_color1.png");
		__S_Device(jet_propeller_color2, "jet_propeller_color2.png");
		__S_Device(jet_propeller_needle, "jet_propeller_needle.png");
		__S_Device(jet_propeller_bottom, "jet_propeller_bottom.png");

#undef __S_GUI
#undef __S_Device

		for(auto & spr : gGUIStaticSpriteMap) {
			SetSpriteOffsetToCenter(spr.second);
		}
		for(auto & spr : gDeviceStaticSpriteMap) {
			SetSpriteOffsetToCenter(spr.second);
		}

		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color1]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color2]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_bottom]->SetOffset({ 108.f, 64.f });
	}

}