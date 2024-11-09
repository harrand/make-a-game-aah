#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP
#include "deck.hpp"
#include "prefab.hpp"

namespace game
{
	void player_setup(game::prefab prefab);
	void player_update(float delta_seconds);
	void player_set_creature(game::prefab prefab);

	deck_handle player_deck();

	unsigned int player_get_max_mana();
	void player_set_max_mana(unsigned int mana);

	unsigned int player_get_mana();
	void player_set_mana(unsigned int mana);
}

#endif // GAME_PLAYER_HPP