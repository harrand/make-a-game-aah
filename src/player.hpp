#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP
#include "deck.hpp"

namespace game
{
	void player_setup();
	void player_update(float delta_seconds);

	deck_handle player_deck();

	unsigned int player_get_max_mana();
	void player_set_max_mana(unsigned int mana);

	unsigned int player_get_mana();
	void player_set_mana(unsigned int mana);
}

#endif // GAME_PLAYER_HPP