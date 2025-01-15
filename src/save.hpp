#ifndef GAME_SAVE_HPP
#define GAME_SAVE_HPP
#include "player.hpp"

namespace game
{
	struct save_data
	{
		player_prefab prefab;
		unsigned int gold;
		std::vector<std::string> completed_levels;
	};

	void save();

	void load_save();
}

#endif // GAME_SAVE_HPP
