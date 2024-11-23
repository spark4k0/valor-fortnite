#pragma once

#include <cstdint>
#include "keybind.hpp"

namespace settings {
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	namespace caching {
		uintptr_t base_address = 0;
	}

	inline int tab = 0;

	namespace aimbot
	{
		inline bool enable = false;
		inline bool show_fov = false;

		inline float fov = 50;
		inline float smoothness_x = 1.55;
		inline float smoothness_y = 1.55;
		inline float thickness = 1;
		inline float BOXthickness = 1;
		inline float OUTthickness = 1;

		inline bool dynamic_prediction = false;
		inline bool enable_smoothness = false;
		inline bool closest_part = false;
		inline bool disable_outside_fov = false;
		inline bool streamer = false;

		inline CKeybind aimbotkey{ "shift"};

		inline int aimbot_type = 0;
		static const char* aimbot_types[2] = { "mouse", "camera" };

		inline int aimbot_part = 0;
		static const char* aimbot_parts[3] = { "head", "neck", "torso" };
	}

	namespace triggerbot {
		inline bool enable_triggerbot = false;
		inline int triggerbot_delay = 1;
		inline int triggerbot_distance = 20;
	}

	namespace visuals
	{
		inline bool enable = false;

		inline bool box = false;
		inline int box_type = 0;
		static const char* box_types[5] = { "2d Bounding", "2d Bounding Corner", "2d Box", "2d Corner Box", "2d Filled" };

		inline bool line = false;
		inline bool skeleton = false;
		inline bool outlines = false;
		inline float SkeletonColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float UsernameColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float SnaplinesColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float DistanceColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float WeaponColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float RankColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float DeviceColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline bool distance = false;
		inline bool username = false;
		inline bool weapon = false;
		inline bool platform = false;
		inline bool rank = false;
	}

	namespace checks
	{
		// checks
		inline bool visible_check = false;
		inline bool death_check = false;
		inline bool team_check = false;
	}

	namespace exploits {
		inline bool magic_bullet = false;
		inline bool no_spread = false;
	}

	namespace overlay 
	{
		inline int particle_type = 0;
		static const char* particle_types[2] = { "Rain", "Snow" };

		inline bool show_menu = true;
	}
}