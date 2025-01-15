#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP
#include "tz/core/vector.hpp"
#include <string>
#include <cstdint>
#include <functional>

namespace game
{
	struct level
	{
		std::string name;
		std::string display_name = "";
		std::string description = "";
		std::uint32_t background_image = -1;
		std::string player_prefab = "";
		std::string prerequisite = "";
		tz::v2f map_position = tz::v2f::filled(-1.0f);
	};

	void level_setup();
	const level& get_level(const std::string& name);
	void iterate_levels(std::function<void(std::string_view, const level&)> callback);

	void clear_level();
	void load_level(const level& l);
	void reload_level();
	const level& get_current_level();
}

#endif // GAME_LEVEL_HPP
