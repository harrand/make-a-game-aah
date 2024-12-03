#ifndef GAME_ENEMY_HPP
#define GAME_ENEMY_HPP 
#include "deck.hpp"
#include "prefab.hpp"

namespace game
{
	void enemy_setup(game::prefab prefab);
	void enemy_update(float delta_seconds);
	void enemy_set_creature(game::prefab prefab);

	deck_handle enemy_deck();
	void enemy_set_pool(std::span<const card> cards);

	unsigned int enemy_get_max_mana();
	void enemy_set_max_mana(unsigned int mana);

	unsigned int enemy_get_mana();
	void enemy_set_mana(unsigned int mana);

	entity_handle enemy_get_avatar();

	bool enemy_try_spend_mana(unsigned int cost);
}

#endif // GAME_ENEMY_HPP
