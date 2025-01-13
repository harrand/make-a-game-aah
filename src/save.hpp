#ifndef GAME_SAVE_HPP
#define GAME_SAVE_HPP
#include "player.hpp"

namespace game
{
	struct save_data
	{
		player_prefab prefab;
	};

	void save();

	void load_save();
}

#endif // GAME_SAVE_HPP
