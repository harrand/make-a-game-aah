#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP
#include <string>
#include <cstdint>

namespace game
{
	struct level
	{
		std::string name;
		std::string display_name = "";
		std::string description = "";
		std::uint32_t background_image = -1;
	};

	void level_setup();
	const level& get_level(const std::string& name);

	void load_level(const level& l);
}

#endif // GAME_LEVEL_HPP
