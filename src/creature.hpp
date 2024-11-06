#ifndef GAME_CREATURE_HPP
#define GAME_CREATURE_HPP
#include "render.hpp"
#include <string>

namespace game
{
	struct creature_prefab
	{
		std::string name;
		unsigned int base_health = 1;
		game::render::flipbook_handle idle = tz::nullhand;
		game::render::flipbook_handle move_horizontal = tz::nullhand;
		game::render::flipbook_handle move_up = tz::nullhand;
		game::render::flipbook_handle move_down = tz::nullhand;
	};

	void creature_setup();
	const creature_prefab& get_creature_prefab(const std::string& name);
}

#endif // GAME_CREATURE_HPP