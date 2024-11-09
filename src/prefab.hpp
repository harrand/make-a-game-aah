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
		game::render::flipbook_handle idle = tz::nullhand;
		game::render::flipbook_handle move_horizontal = tz::nullhand;
		game::render::flipbook_handle move_up = tz::nullhand;
		game::render::flipbook_handle move_down = tz::nullhand;
		game::render::flipbook_handle cast = tz::nullhand;
	};

	void prefab_setup();
	const prefab& get_prefab(const std::string& name);
}

#endif // GAME_PREFAB_HPP