#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP
#include "entity.hpp"
#include "deck.hpp"
#include "prefab.hpp"

namespace game
{
	void player_setup(game::prefab prefab);
	void player_update(float delta_seconds);
	void player_set_creature(game::prefab prefab);

	deck_handle player_deck();
	void player_set_pool(std::span<const card> cards);

	unsigned int player_get_max_mana();
	void player_set_max_mana(unsigned int mana);

	unsigned int player_get_mana();
	void player_set_mana(unsigned int mana);

	float player_get_mps();
	void player_set_mps(float mps);

	bool player_targets(entity_handle ent);
	void player_drop_target_entity();

	entity_handle player_get_avatar();
	void player_on_death();

	entity_handle player_get_target();
	std::optional<tz::v2f> player_get_target_location();

	bool player_try_spend_mana(unsigned int cost);
}

#endif // GAME_PLAYER_HPP
