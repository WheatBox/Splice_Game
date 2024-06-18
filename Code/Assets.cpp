#include "Assets.h"

namespace Assets {

	std::unordered_map<EGUIStaticSprite, Frame::CStaticSprite *> gGUIStaticSpriteMap {};
	std::unordered_map<EDeviceStaticSprite, Frame::CStaticSprite *> gDeviceStaticSpriteMap {};
	std::unordered_map<EOtherStaticSprite, Frame::CStaticSprite *> gOtherStaticSpriteMap {};

	static void SetSpriteOffsetToCenter(Frame::ISprite * pSpr) {
		pSpr->SetOffset({ .5f * static_cast<float>(pSpr->GetWidth()), .5f * static_cast<float>(pSpr->GetHeight()) });
	}

	void LoadPermanentAssets() {
		Frame::CTextureAtlas * pGUIAtlas = new Frame::CTextureAtlas {
			"Assets/Art/atlas_GUI.png",
			{ // "image name" : (left top) (right bottom)
				{ "tool_controller.png", { { 0.000000f, 1.000000f }, { 0.187500f, 0.812500f } } },
				{ "tool_eraser.png", { { 0.000000f, 0.812500f }, { 0.187500f, 0.625000f } } },
				{ "tool_hand.png", { { 0.000000f, 0.625000f }, { 0.187500f, 0.437500f } } },
				{ "tool_pencil.png", { { 0.000000f, 0.437500f }, { 0.187500f, 0.250000f } } },
				{ "tool_pipe.png", { { 0.000000f, 0.250000f }, { 0.187500f, 0.062500f } } },
				{ "tool_pipe_mode_eraser.png", { { 0.187500f, 1.000000f }, { 0.375000f, 0.812500f } } },
				{ "tool_pipe_mode_insert.png", { { 0.187500f, 0.812500f }, { 0.375000f, 0.625000f } } },
				{ "tool_pipe_mode_pencil.png", { { 0.187500f, 0.625000f }, { 0.375000f, 0.437500f } } },
				{ "tool_swatches.png", { { 0.187500f, 0.437500f }, { 0.375000f, 0.250000f } } }
			}
		};
		Frame::CTextureAtlas * pDevicesAtlas = new Frame::CTextureAtlas {
			"Assets/Art/atlas_Devices.png",
			{ // "image name" : (left top) (right bottom)
				{ "propeller_blade_color.png", { { 0.003906f, 0.996094f }, { 0.128906f, 0.871094f } } },
				{ "propeller_blade.png", { { 0.003906f, 0.867188f }, { 0.128906f, 0.742188f } } },
				{ "jet_propeller_color2.png", { { 0.003906f, 0.738281f }, { 0.128906f, 0.675781f } } },
				{ "jet_propeller_color1.png", { { 0.003906f, 0.671875f }, { 0.128906f, 0.609375f } } },
				{ "jet_propeller_bottom.png", { { 0.003906f, 0.605469f }, { 0.128906f, 0.542969f } } },
				{ "jet_propeller.png", { { 0.003906f, 0.539063f }, { 0.128906f, 0.476563f } } },
				{ "propeller_motor_color2.png", { { 0.003906f, 0.472656f }, { 0.066406f, 0.347656f } } },
				{ "propeller_top.png", { { 0.003906f, 0.343750f }, { 0.066406f, 0.218750f } } },
				{ "propeller_motor.png", { { 0.003906f, 0.214844f }, { 0.066406f, 0.089844f } } },
				{ "propeller_motor_color1.png", { { 0.070313f, 0.472656f }, { 0.132813f, 0.347656f } } },
				{ "propeller_top_color.png", { { 0.070313f, 0.343750f }, { 0.132813f, 0.218750f } } },
				{ "cabin_color2.png", { { 0.003906f, 0.085938f }, { 0.066406f, 0.023438f } } },
				{ "cabin_color1.png", { { 0.070313f, 0.214844f }, { 0.132813f, 0.152344f } } },
				{ "cabin.png", { { 0.070313f, 0.148438f }, { 0.132813f, 0.085938f } } },
				{ "shell_color2.png", { { 0.070313f, 0.082031f }, { 0.132813f, 0.019531f } } },
				{ "engine_color2.png", { { 0.132813f, 0.996094f }, { 0.195313f, 0.933594f } } },
				{ "joint.png", { { 0.132813f, 0.929688f }, { 0.195313f, 0.867188f } } },
				{ "joint_bottom.png", { { 0.132813f, 0.863281f }, { 0.195313f, 0.800781f } } },
				{ "joint_color.png", { { 0.132813f, 0.796875f }, { 0.195313f, 0.734375f } } },
				{ "joint_top.png", { { 0.132813f, 0.730469f }, { 0.195313f, 0.667969f } } },
				{ "joint_top_color.png", { { 0.132813f, 0.664063f }, { 0.195313f, 0.601563f } } },
				{ "engine_color1.png", { { 0.132813f, 0.597656f }, { 0.195313f, 0.535156f } } },
				{ "engine.png", { { 0.136719f, 0.531250f }, { 0.199219f, 0.468750f } } },
				{ "shell_color1.png", { { 0.136719f, 0.464844f }, { 0.199219f, 0.402344f } } },
				{ "cabin_logo_background.png", { { 0.136719f, 0.398438f }, { 0.199219f, 0.335938f } } },
				{ "shell.png", { { 0.136719f, 0.332031f }, { 0.199219f, 0.269531f } } },
				{ "pipe_cross_color.png", { { 0.136719f, 0.265625f }, { 0.160156f, 0.242188f } } },
				{ "pipe_bend.png", { { 0.136719f, 0.238281f }, { 0.160156f, 0.214844f } } },
				{ "pipe_bend_color.png", { { 0.136719f, 0.210938f }, { 0.160156f, 0.187500f } } },
				{ "pipe_junction_color.png", { { 0.136719f, 0.183594f }, { 0.160156f, 0.160156f } } },
				{ "pipe_junction.png", { { 0.136719f, 0.156250f }, { 0.160156f, 0.132813f } } },
				{ "pipe_cross.png", { { 0.136719f, 0.128906f }, { 0.160156f, 0.105469f } } },
				{ "jet_propeller_needle.png", { { 0.003906f, 0.019531f }, { 0.027344f, 0.011719f } } },
				{ "connector.png", { { 0.136719f, 0.101563f }, { 0.152344f, 0.039063f } } },
				{ "pipe_interface.png", { { 0.031250f, 0.019531f }, { 0.046875f, 0.003906f } } },
				{ "pipe.png", { { 0.050781f, 0.019531f }, { 0.066406f, 0.003906f } } },
				{ "pipe_interface_color.png", { { 0.136719f, 0.035156f }, { 0.152344f, 0.019531f } } },
				{ "pipe_color.png", { { 0.156250f, 0.101563f }, { 0.171875f, 0.085938f } } },
				{ "pipe_joint.png", { { 0.156250f, 0.082031f }, { 0.171875f, 0.066406f } } },
				{ "pipe_joint_color.png", { { 0.156250f, 0.062500f }, { 0.171875f, 0.046875f } } }
			}
		};
		Frame::CTextureAtlas * pOthersAtlas = new Frame::CTextureAtlas {
			"Assets/Art/atlas_Others.png",
			{ // "image name" : (left top) (right bottom)
				{ "smoke2.png", { { 0.000000f, 1.000000f }, { 0.250000f, 0.750000f } } },
				{ "smoke3.png", { { 0.000000f, 0.750000f }, { 0.250000f, 0.500000f } } },
				{ "smoke4.png", { { 0.000000f, 0.500000f }, { 0.250000f, 0.250000f } } },
				{ "smoke5.png", { { 0.000000f, 0.250000f }, { 0.250000f, 0.000000f } } },
				{ "smoke1.png", { { 0.250000f, 1.000000f }, { 0.500000f, 0.750000f } } }
			}
		};

#define __S_GUI(_e, _uvKey) gGUIStaticSpriteMap[EGUIStaticSprite::_e] = new Frame::CStaticSprite { pGUIAtlas, _uvKey };

		__S_GUI(Editor_tool_hand, "tool_hand.png");
		__S_GUI(Editor_tool_pencil, "tool_pencil.png");
		__S_GUI(Editor_tool_eraser, "tool_eraser.png");
		__S_GUI(Editor_tool_pipe, "tool_pipe.png");
		__S_GUI(Editor_tool_swatches, "tool_swatches.png");
		__S_GUI(Editor_tool_controller, "tool_controller.png");
		__S_GUI(Editor_tool_pipe_mode_pencil, "tool_pipe_mode_pencil.png");
		__S_GUI(Editor_tool_pipe_mode_eraser, "tool_pipe_mode_eraser.png");
		__S_GUI(Editor_tool_pipe_mode_insert, "tool_pipe_mode_insert.png");

#define __S_Device(_e, _uvKey) gDeviceStaticSpriteMap[EDeviceStaticSprite::_e] = new Frame::CStaticSprite { pDevicesAtlas, _uvKey };

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
		__S_Device(joint, "joint.png");
		__S_Device(joint_color, "joint_color.png");
		__S_Device(joint_top, "joint_top.png");
		__S_Device(joint_top_color, "joint_top_color.png");
		__S_Device(joint_bottom, "joint_bottom.png");

#define __S_Other(_e, _uvKey) gOtherStaticSpriteMap[EOtherStaticSprite::_e] = new Frame::CStaticSprite { pOthersAtlas, _uvKey };

		__S_Other(smoke1, "smoke1.png");
		__S_Other(smoke2, "smoke2.png");
		__S_Other(smoke3, "smoke3.png");
		__S_Other(smoke4, "smoke4.png");
		__S_Other(smoke5, "smoke5.png");

#undef __S_GUI
#undef __S_Device
#undef __S_Other

		for(auto & spr : gGUIStaticSpriteMap) {
			SetSpriteOffsetToCenter(spr.second);
		}
		for(auto & spr : gDeviceStaticSpriteMap) {
			SetSpriteOffsetToCenter(spr.second);
		}
		for(auto & spr : gOtherStaticSpriteMap) {
			SetSpriteOffsetToCenter(spr.second);
		}

		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color1]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color2]->SetOffset({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_bottom]->SetOffset({ 108.f, 64.f });
	}

}