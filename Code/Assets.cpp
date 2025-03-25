#include "Assets.h"

namespace Assets {

	std::unordered_map<EGUIStaticSprite, Frame::CStaticSprite *> gGUIStaticSpriteMap {};
	std::unordered_map<EDeviceStaticSprite, Frame::CStaticSprite *> gDeviceStaticSpriteMap {};
	std::unordered_map<EOtherStaticSprite, Frame::CStaticSprite *> gOtherStaticSpriteMap {};

	std::unordered_map<const Frame::SSpriteImage *, Frame::CRenderer::SInstanceBuffer> gSpriteImageInstanceBufferMap {};

	static void SetSpriteOriginToCenter(Frame::ISprite * pSpr) {
		pSpr->SetOrigin({ .5f * static_cast<float>(pSpr->GetWidth()), .5f * static_cast<float>(pSpr->GetHeight()) });
	}

	void LoadPermanentAssets() {
		Frame::CTextureAtlas * pGUIAtlas = new Frame::CTextureAtlas {
			"Assets/Art/atlas_GUI.png",
			{ // "image name" : (left top) (right bottom)
				{ "tool_swatches.png", { { 0.000000f, 1.000000f }, { 0.187500f, 0.812500f } } },
				{ "tool_controller.png", { { 0.000000f, 0.812500f }, { 0.187500f, 0.625000f } } },
				{ "tool_eraser.png", { { 0.000000f, 0.625000f }, { 0.187500f, 0.437500f } } },
				{ "tool_hand.png", { { 0.000000f, 0.437500f }, { 0.187500f, 0.250000f } } },
				{ "tool_hand_heart.png", { { 0.000000f, 0.250000f }, { 0.187500f, 0.062500f } } },
				{ "tool_pencil.png", { { 0.187500f, 1.000000f }, { 0.375000f, 0.812500f } } },
				{ "tool_pipe.png", { { 0.187500f, 0.812500f }, { 0.375000f, 0.625000f } } },
				{ "tool_pipe_mode_eraser.png", { { 0.187500f, 0.625000f }, { 0.375000f, 0.437500f } } },
				{ "tool_pipe_mode_insert.png", { { 0.187500f, 0.437500f }, { 0.375000f, 0.250000f } } },
				{ "tool_pipe_mode_pencil.png", { { 0.187500f, 0.250000f }, { 0.375000f, 0.062500f } } },
				{ "controller_button_pressing.png", { { 0.375000f, 1.000000f }, { 0.500000f, 0.843750f } } },
				{ "controller_button_free.png", { { 0.375000f, 0.843750f }, { 0.500000f, 0.687500f } } }
			}
		};
		Frame::CTextureAtlas * pDevicesAtlas = new Frame::CTextureAtlas {
			"Assets/Art/atlas_Devices.png",
			{ // "image name" : (left top) (right bottom)
				{ "propeller_blade.png", { { 0.001953f, 0.998047f }, { 0.126953f, 0.873047f } } },
				{ "propeller_blade_color.png", { { 0.001953f, 0.871094f }, { 0.126953f, 0.746094f } } },
				{ "jet_propeller_bottom.png", { { 0.001953f, 0.744141f }, { 0.126953f, 0.681641f } } },
				{ "jet_propeller.png", { { 0.001953f, 0.679688f }, { 0.126953f, 0.617188f } } },
				{ "jet_propeller_color2.png", { { 0.001953f, 0.615234f }, { 0.126953f, 0.552734f } } },
				{ "jet_propeller_color1.png", { { 0.001953f, 0.550781f }, { 0.126953f, 0.488281f } } },
				{ "propeller_motor_color1.png", { { 0.001953f, 0.486328f }, { 0.064453f, 0.361328f } } },
				{ "propeller_top_color.png", { { 0.001953f, 0.359375f }, { 0.064453f, 0.234375f } } },
				{ "propeller_top.png", { { 0.001953f, 0.232422f }, { 0.064453f, 0.107422f } } },
				{ "propeller_motor_color2.png", { { 0.066406f, 0.486328f }, { 0.128906f, 0.361328f } } },
				{ "propeller_motor.png", { { 0.066406f, 0.359375f }, { 0.128906f, 0.234375f } } },
				{ "engine_color2.png", { { 0.001953f, 0.105469f }, { 0.064453f, 0.042969f } } },
				{ "shell.png", { { 0.066406f, 0.232422f }, { 0.128906f, 0.169922f } } },
				{ "jet_propeller_smoke1.png", { { 0.066406f, 0.167969f }, { 0.128906f, 0.105469f } } },
				{ "engine_color1.png", { { 0.066406f, 0.103516f }, { 0.128906f, 0.041016f } } },
				{ "jet_propeller_smoke3.png", { { 0.128906f, 0.998047f }, { 0.191406f, 0.935547f } } },
				{ "jet_propeller_smoke4.png", { { 0.128906f, 0.933594f }, { 0.191406f, 0.871094f } } },
				{ "jet_propeller_smoke5.png", { { 0.128906f, 0.869141f }, { 0.191406f, 0.806641f } } },
				{ "joint.png", { { 0.128906f, 0.804688f }, { 0.191406f, 0.742188f } } },
				{ "joint_bottom.png", { { 0.128906f, 0.740234f }, { 0.191406f, 0.677734f } } },
				{ "joint_color.png", { { 0.128906f, 0.675781f }, { 0.191406f, 0.613281f } } },
				{ "joint_top.png", { { 0.128906f, 0.611328f }, { 0.191406f, 0.548828f } } },
				{ "joint_top_color.png", { { 0.130859f, 0.546875f }, { 0.193359f, 0.484375f } } },
				{ "engine.png", { { 0.130859f, 0.482422f }, { 0.193359f, 0.419922f } } },
				{ "shell_color1.png", { { 0.130859f, 0.417969f }, { 0.193359f, 0.355469f } } },
				{ "cabin_logo_background.png", { { 0.130859f, 0.353516f }, { 0.193359f, 0.291016f } } },
				{ "cabin_color1.png", { { 0.130859f, 0.289063f }, { 0.193359f, 0.226563f } } },
				{ "cabin_color2.png", { { 0.130859f, 0.224609f }, { 0.193359f, 0.162109f } } },
				{ "jet_propeller_smoke2.png", { { 0.130859f, 0.160156f }, { 0.193359f, 0.097656f } } },
				{ "cabin.png", { { 0.130859f, 0.095703f }, { 0.193359f, 0.033203f } } },
				{ "shell_color2.png", { { 0.193359f, 0.998047f }, { 0.255859f, 0.935547f } } },
				{ "pipe_bend.png", { { 0.001953f, 0.041016f }, { 0.025391f, 0.017578f } } },
				{ "pipe_bend_color.png", { { 0.027344f, 0.041016f }, { 0.050781f, 0.017578f } } },
				{ "pipe_junction.png", { { 0.052734f, 0.039063f }, { 0.076172f, 0.015625f } } },
				{ "pipe_junction_color.png", { { 0.078125f, 0.039063f }, { 0.101563f, 0.015625f } } },
				{ "pipe_cross.png", { { 0.103516f, 0.039063f }, { 0.126953f, 0.015625f } } },
				{ "pipe_cross_color.png", { { 0.128906f, 0.031250f }, { 0.152344f, 0.007813f } } },
				{ "jet_propeller_needle.png", { { 0.001953f, 0.015625f }, { 0.025391f, 0.007813f } } },
				{ "connector.png", { { 0.193359f, 0.933594f }, { 0.208984f, 0.871094f } } },
				{ "pipe_joint.png", { { 0.154297f, 0.031250f }, { 0.169922f, 0.015625f } } },
				{ "pipe.png", { { 0.171875f, 0.031250f }, { 0.187500f, 0.015625f } } },
				{ "pipe_joint_color.png", { { 0.189453f, 0.031250f }, { 0.205078f, 0.015625f } } },
				{ "pipe_color.png", { { 0.193359f, 0.869141f }, { 0.208984f, 0.853516f } } },
				{ "pipe_interface_color.png", { { 0.193359f, 0.851563f }, { 0.208984f, 0.835938f } } },
				{ "pipe_interface.png", { { 0.193359f, 0.833984f }, { 0.208984f, 0.818359f } } }
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

		__S_GUI(Controller_button_pressing, "controller_button_pressing.png");
		__S_GUI(Controller_button_free, "controller_button_free.png");

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
		__S_Device(jet_propeller_smoke1, "jet_propeller_smoke1.png");
		__S_Device(jet_propeller_smoke2, "jet_propeller_smoke2.png");
		__S_Device(jet_propeller_smoke3, "jet_propeller_smoke3.png");
		__S_Device(jet_propeller_smoke4, "jet_propeller_smoke4.png");
		__S_Device(jet_propeller_smoke5, "jet_propeller_smoke5.png");
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

		static auto generateStaticSpritesInstanceBuffers = [](const Frame::CStaticSprite * spr) {
			gSpriteImageInstanceBufferMap.insert({ spr->GetImage(), Frame::CRenderer::SInstanceBuffer { Frame::Matrix33::CreateScale(Frame::Vec2Cast(spr->GetSize())) }.SetUVTransformation(spr->GetImage()->GetUVLeftTop(), spr->GetImage()->GetUVRightBottom()) });
		};

		for(auto & spr : gGUIStaticSpriteMap) {
			SetSpriteOriginToCenter(spr.second);
			generateStaticSpritesInstanceBuffers(spr.second);
		}
		for(auto & spr : gDeviceStaticSpriteMap) {
			SetSpriteOriginToCenter(spr.second);
			generateStaticSpritesInstanceBuffers(spr.second);
		}
		for(auto & spr : gOtherStaticSpriteMap) {
			SetSpriteOriginToCenter(spr.second);
			generateStaticSpritesInstanceBuffers(spr.second);
		}

		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller]->SetOrigin({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color1]->SetOrigin({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_color2]->SetOrigin({ 108.f, 64.f });
		gDeviceStaticSpriteMap[EDeviceStaticSprite::jet_propeller_bottom]->SetOrigin({ 108.f, 64.f });
	}

}
