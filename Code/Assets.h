#pragma once

#include <FrameAsset/Sprite.h>
#include <FrameAsset/Font.h>

#include <unordered_map>

#include "DevicesData.h"

namespace Assets {

	static inline const char * GetFontFilename() {
		return "Assets/Fonts/975SCRegular.ttf";
	}

	enum class EGUIStaticSprite {
		UNKNOWN,
		Editor_tool_hand,
		Editor_tool_pencil,
		Editor_tool_eraser,
		Editor_tool_pipe,
		Editor_tool_swatches,
		Editor_tool_controller,
		Editor_tool_pipe_mode_pencil,
		Editor_tool_pipe_mode_eraser,
		Editor_tool_pipe_mode_insert,
	};

	enum class EDeviceStaticSprite {
		UNKNOWN,
		connector,
		pipe,
		pipe_color,
		pipe_joint,
		pipe_joint_color,
		pipe_bend,
		pipe_bend_color,
		pipe_junction,
		pipe_junction_color,
		pipe_cross,
		pipe_cross_color,
		pipe_interface,
		pipe_interface_color,
		cabin,
		cabin_color1,
		cabin_color2,
		cabin_logo_background,
		shell,
		shell_color1,
		shell_color2,
		engine,
		engine_color1,
		engine_color2,
		propeller_motor,
		propeller_motor_color1,
		propeller_motor_color2,
		propeller_top,
		propeller_top_color,
		propeller_blade,
		propeller_blade_color,
		jet_propeller,
		jet_propeller_color1,
		jet_propeller_color2,
		jet_propeller_needle,
		jet_propeller_bottom,
	};

	enum class EOtherStaticSprite {
		smoke1,
		smoke2,
		smoke3,
		smoke4,
		smoke5,
	};

	extern std::unordered_map<EGUIStaticSprite, Frame::CStaticSprite *> gGUIStaticSpriteMap;
	extern std::unordered_map<EDeviceStaticSprite, Frame::CStaticSprite *> gDeviceStaticSpriteMap;
	extern std::unordered_map<EOtherStaticSprite, Frame::CStaticSprite *> gOtherStaticSpriteMap;

	// 加载永久资产，这类资产因为需要经常使用，因此在游戏开始时就加载进内存中，且在整个游戏运行期间内不会进行释放
	void LoadPermanentAssets();

#define DefineFunction_GetStaticSprite(_enum_type_name, _map_name) \
	static inline Frame::CStaticSprite * GetStaticSprite(_enum_type_name spr) { \
		if(auto it = _map_name.find(spr); it != _map_name.end()) { \
			return it->second; \
		} \
		return nullptr; \
	}

	DefineFunction_GetStaticSprite(EGUIStaticSprite, gGUIStaticSpriteMap)
	DefineFunction_GetStaticSprite(EDeviceStaticSprite, gDeviceStaticSpriteMap)
	DefineFunction_GetStaticSprite(EOtherStaticSprite, gOtherStaticSpriteMap)

#undef DefineFunction_GetStaticSprite

	enum class EDeviceStaticSpritePart {
		basic,
		color1,
		color2
	};

	static inline Frame::CStaticSprite * GetDeviceStaticSprite(IDeviceData::EType type, EDeviceStaticSpritePart part) {
		static constexpr EDeviceStaticSprite sprs[] = {
			EDeviceStaticSprite::UNKNOWN, // Unset
			EDeviceStaticSprite::UNKNOWN,
			EDeviceStaticSprite::UNKNOWN,
			EDeviceStaticSprite::cabin, // Cabin
			EDeviceStaticSprite::cabin_color1,
			EDeviceStaticSprite::cabin_color2,
			EDeviceStaticSprite::shell, // Shell
			EDeviceStaticSprite::shell_color1,
			EDeviceStaticSprite::shell_color2,
			EDeviceStaticSprite::engine, // Engine
			EDeviceStaticSprite::engine_color1,
			EDeviceStaticSprite::engine_color2,
			EDeviceStaticSprite::propeller_motor, // Propeller
			EDeviceStaticSprite::propeller_motor_color1,
			EDeviceStaticSprite::propeller_motor_color2,
			EDeviceStaticSprite::jet_propeller, // Jet Propeller
			EDeviceStaticSprite::jet_propeller_color1,
			EDeviceStaticSprite::jet_propeller_color2,
			EDeviceStaticSprite::UNKNOWN, // END
			EDeviceStaticSprite::UNKNOWN,
			EDeviceStaticSprite::UNKNOWN,
		};
		return GetStaticSprite(sprs[type * 3 + static_cast<int>(part)]);
	}

}