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
				{ "propeller_blade_color.png", { { 0.000000f, 1.000000f }, { 0.125000f, 0.875000f } } },
				{ "propeller_blade.png", { { 0.000000f, 0.875000f }, { 0.125000f, 0.750000f } } },
				{ "jet_propeller_color2.png", { { 0.000000f, 0.750000f }, { 0.125000f, 0.687500f } } },
				{ "jet_propeller_color1.png", { { 0.000000f, 0.687500f }, { 0.125000f, 0.625000f } } },
				{ "jet_propeller_bottom.png", { { 0.000000f, 0.625000f }, { 0.125000f, 0.562500f } } },
				{ "jet_propeller.png", { { 0.000000f, 0.562500f }, { 0.125000f, 0.500000f } } },
				{ "propeller_top_color.png", { { 0.000000f, 0.500000f }, { 0.062500f, 0.375000f } } },
				{ "propeller_motor_color1.png", { { 0.000000f, 0.375000f }, { 0.062500f, 0.250000f } } },
				{ "propeller_motor_color2.png", { { 0.000000f, 0.250000f }, { 0.062500f, 0.125000f } } },
				{ "propeller_top.png", { { 0.000000f, 0.125000f }, { 0.062500f, 0.000000f } } },
				{ "propeller_motor.png", { { 0.062500f, 0.500000f }, { 0.125000f, 0.375000f } } },
				{ "cabin_logo_background.png", { { 0.062500f, 0.375000f }, { 0.125000f, 0.312500f } } },
				{ "engine_color1.png", { { 0.062500f, 0.312500f }, { 0.125000f, 0.250000f } } },
				{ "shell_color1.png", { { 0.062500f, 0.250000f }, { 0.125000f, 0.187500f } } },
				{ "engine.png", { { 0.062500f, 0.187500f }, { 0.125000f, 0.125000f } } },
				{ "engine_color2.png", { { 0.062500f, 0.125000f }, { 0.125000f, 0.062500f } } },
				{ "shell_color2.png", { { 0.062500f, 0.062500f }, { 0.125000f, 0.000000f } } },
				{ "shell.png", { { 0.125000f, 1.000000f }, { 0.187500f, 0.937500f } } },
				{ "cabin_color2.png", { { 0.125000f, 0.937500f }, { 0.187500f, 0.875000f } } },
				{ "cabin_color1.png", { { 0.125000f, 0.875000f }, { 0.187500f, 0.812500f } } },
				{ "cabin.png", { { 0.125000f, 0.812500f }, { 0.187500f, 0.750000f } } },
				{ "pipe_cross.png", { { 0.125000f, 0.750000f }, { 0.148438f, 0.726563f } } },
				{ "pipe_bend.png", { { 0.125000f, 0.726563f }, { 0.148438f, 0.703125f } } },
				{ "pipe_junction.png", { { 0.125000f, 0.703125f }, { 0.148438f, 0.679688f } } },
				{ "pipe_junction_color.png", { { 0.125000f, 0.679688f }, { 0.148438f, 0.656250f } } },
				{ "pipe_bend_color.png", { { 0.125000f, 0.656250f }, { 0.148438f, 0.632813f } } },
				{ "pipe_cross_color.png", { { 0.125000f, 0.632813f }, { 0.148438f, 0.609375f } } },
				{ "jet_propeller_needle.png", { { 0.125000f, 0.609375f }, { 0.148438f, 0.601563f } } },
				{ "connector.png", { { 0.125000f, 0.601563f }, { 0.140625f, 0.539063f } } },
				{ "pipe_joint_color.png", { { 0.125000f, 0.539063f }, { 0.140625f, 0.523438f } } },
				{ "pipe.png", { { 0.125000f, 0.523438f }, { 0.140625f, 0.507813f } } },
				{ "pipe_interface_color.png", { { 0.125000f, 0.507813f }, { 0.140625f, 0.492188f } } },
				{ "pipe_color.png", { { 0.125000f, 0.492188f }, { 0.140625f, 0.476563f } } },
				{ "pipe_interface.png", { { 0.125000f, 0.476563f }, { 0.140625f, 0.460938f } } },
				{ "pipe_joint.png", { { 0.125000f, 0.460938f }, { 0.140625f, 0.445313f } } }
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