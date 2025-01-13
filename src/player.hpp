#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP
#include "entity.hpp"
#include "deck.hpp"
#include "prefab.hpp"

namespace game
{
	namespace detail{ struct player_t{}; }
	using player_handle = tz::handle<detail::player_t>;
	enum class player_type
	{
		human,
		cpu
	};

	void iterate_players(std::function<void(player_handle)> callback);
	void iterate_players_if(std::function<void(player_handle)> callback, std::function<bool(player_handle)> predicate);
	
	player_handle create_player(player_type type, bool good, game::prefab prefab);
	void destroy_player(player_handle p);
	void clear_players();

	void player_update(float delta_seconds);
	bool player_is_good(player_handle p);
	bool player_is_human(player_handle p);

	deck_handle player_deck(player_handle p);
	void player_set_pool(player_handle p, std::span<const card> cards);

	unsigned int player_get_max_mana(player_handle p);
	void player_set_max_mana(player_handle p, unsigned int mana);

	unsigned int player_get_mana(player_handle p);
	void player_set_mana(player_handle p, unsigned int mana);

	float player_get_mps(player_handle p);
	void player_set_mps(player_handle p, float mps);

	bool player_targets(player_handle p, entity_handle ent);
	void player_drop_target_entity(player_handle p);
	bool player_owns(player_handle p, entity_handle e);

	void player_control_entity(player_handle p, entity_handle e);
	player_handle try_get_player_that_controls_entity(entity_handle e);

	entity_handle player_get_avatar(player_handle p);
	void player_on_death(player_handle p);

	entity_handle player_get_target(player_handle p);
	std::optional<tz::v2f> player_get_target_location(player_handle p);

	bool player_try_spend_mana(player_handle p, unsigned int cost);

	struct player_prefab
	{
		std::string name;
		std::string avatar_prefab;
		std::vector<std::string> deck;
	};

	void player_setup();

	player_prefab& get_player_prefab(const std::string& name);
	player_handle load_player_prefab(const player_prefab& prefab, bool cpu = true, bool player_aligned = false);
}

#endif // GAME_PLAYER_HPP
