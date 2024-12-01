#ifndef GAME_PREFAB_HPP
#define GAME_PREFAB_HPP
#include "render.hpp"
#include <string>

namespace game
{
	struct prefab
	{
		std::string name;
		float movement_speed;
		unsigned int base_health = 1;
		unsigned int power = 0;
		std::string display_name = "";
		std::string description = "";
		bool has_tooltip = true;
		bool combat = true;
		float base_cooldown = 1.0f;
		float leeway_coefficient = 1.0f;
		bool face_move_direction = false;
		bool spell_decoration = false;
		bool require_target_entity_to_play = false;
		bool require_target_location_to_play = false;
		tz::v3f colour_tint = tz::v3f::filled(1.0f);
		game::render::flipbook_handle idle = tz::nullhand;
		game::render::flipbook_handle move_horizontal = tz::nullhand;
		game::render::flipbook_handle move_up = tz::nullhand;
		game::render::flipbook_handle move_down = tz::nullhand;
		game::render::flipbook_handle cast = tz::nullhand;
		game::render::flipbook_handle attack = tz::nullhand;
	};

	void prefab_setup();
	const prefab& get_prefab(const std::string& name);
}

#endif // GAME_PREFAB_HPP