#include "player.hpp"
#include "config.hpp"
#include "save.hpp"
#include "ui.hpp"
#include "level.hpp"
#include "tz/topaz.hpp"
#include "tz/core/lua.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include <algorithm>
#include <cstdlib>

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	entity_handle reticule = tz::nullhand;
	std::size_t living_good_players = 0;
	std::size_t living_evil_players = 0;

	struct player_data
	{
		bool valid = true;
		player_type type;
		bool good;
		entity_handle avatar = tz::nullhand;
		deck_handle deck;
		unsigned int max_mana = config_default_max_mana;
		float mana = 0;
		float mana_regen = config_default_mps;

		static constexpr tz::v2f mana_bar_pos = {-0.5f, -0.9f};
		static constexpr tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
		std::vector<bool> deck_hold_array = {};

		entity_handle target_entity = tz::nullhand;
		std::optional<tz::v2f> target_location = std::nullopt;

		std::vector<card> card_pool = {};
		std::size_t card_pool_cursor = 0;

		float cpu_play_timer = 0.0f;
		card cpu_play_card = {};
		render::handle cpu_play_card_quad = tz::nullhand;

		tz::v3f get_colour() const
		{
			if(this->good)
			{
				return this->type == player_type::human ? config_player_colour : config_ally_aligned_colour;
			}
			else
			{
				return config_enemy_aligned_colour;
			}
		}
	};

	std::vector<player_data> players = {};
	player_handle human_player = tz::nullhand;

	void impl_update_single_player(player_handle p, float delta_seconds);
	void impl_update_reticule();
	void impl_setup_player(player_handle p, game::prefab prefab);
	bool impl_ensure_no_human_players();
	void impl_update_cpu_player(player_handle p, float delta_seconds);
	void impl_update_human_player();
	void impl_human_handle_deck(player_handle p);
	void impl_cpu_select_card_to_play(player_handle p, std::size_t deck_id);
	bool impl_cpu_try_play_selected_card(player_handle p, float delta_seconds);

	void iterate_players(std::function<void(player_handle)> callback)
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			callback(static_cast<tz::hanval>(i));
		}
	}

	void iterate_players_if(std::function<void(player_handle)> callback, std::function<bool(player_handle)> predicate)
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			player_handle p = static_cast<tz::hanval>(i);
			if(predicate(p))
			{
				callback(p);
			}
		}
	}

	player_handle create_player(player_type type, bool good, game::prefab prefab, float mana_coeff)
	{
		bool recycled = false;
		player_handle ret = static_cast<tz::hanval>(players.size());
		for(std::size_t i = 0; i < players.size(); i++)
		{
			if(!players[i].valid)
			{
				ret = static_cast<tz::hanval>(i);
				recycled = true;
			}
		}

		if(type == player_type::human)
		{
			tz_assert(impl_ensure_no_human_players(), "cannot add more than one human player");
			human_player = ret;
		}

		if(!recycled)
		{
			players.push_back({.type = type, .good = good, .mana_regen = config_default_mps * mana_coeff});
		}
		else
		{
			players[ret.peek()].type = type;
			players[ret.peek()].good = good;
			players[ret.peek()].valid = true;
			players[ret.peek()].mana_regen = config_default_mps * mana_coeff;
		}
		impl_setup_player(ret, prefab);
		if(good)
		{
			living_good_players++;
		}
		else
		{
			living_evil_players++;
		}

		return ret;
	}

	void destroy_player(player_handle p)
	{
		auto& pl = players[p.peek()];
		game::destroy_deck(pl.deck);
		game::render::destroy_quad(pl.mana_bar_background);
		game::render::destroy_quad(pl.mana_bar);
		if(game::entity_exists(pl.avatar))
		{
			game::destroy_entity(pl.avatar);
		}
		if(pl.cpu_play_card_quad != tz::nullhand)
		{
			game::render::destroy_quad(pl.cpu_play_card_quad);
		}
		if(pl.good)
		{
			living_good_players--;
		}
		else
		{
			living_evil_players--;
		}
		pl = {};
		pl.valid = false;
	}

	void clear_players()
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			if(players[i].valid)
			{
				destroy_player(static_cast<tz::hanval>(i));
			}
		}
	}

	void player_update(float delta_seconds)
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			if(players[i].valid)
			{
				impl_update_single_player(static_cast<tz::hanval>(i), delta_seconds);
			}
		}
	}

	bool player_is_good(player_handle p)
	{
		return players[p.peek()].good;
	}

	bool player_is_human(player_handle p)
	{
		return players[p.peek()].type == player_type::human;
	}

	deck_handle player_deck(player_handle p)
	{
		return players[p.peek()].deck;
	}

	void player_set_pool(player_handle p, std::span<const card> cards)
	{
		auto& player = players[p.peek()];
		player.card_pool.clear();
		player.card_pool.resize(cards.size());
		std::copy(cards.begin(), cards.end(), player.card_pool.begin());
		player.card_pool_cursor = 0;
	}

	unsigned int player_get_max_mana(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.max_mana;
	}
	
	void player_set_max_mana(player_handle p, unsigned int mana)
	{
		auto& player = players[p.peek()];
		player.max_mana = mana;
		player_set_mana(p, player.mana);
	}

	unsigned int player_get_mana(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.mana;
	}

	void player_set_mana(player_handle p, unsigned int mana)
	{
		auto& player = players[p.peek()];
		player.mana = std::clamp(mana, 0u, player.max_mana);

		float manapct = static_cast<float>(player.mana) / player.max_mana;
		tz::v2f scale = player.mana_bar_dimensions;
		scale[0] *= manapct;
		game::render::quad_set_scale(player.mana_bar, scale);

		tz::v2f bg_scale = game::render::quad_get_scale(player.mana_bar_background);
		float yoffset = bg_scale[1] - (bg_scale[1] / (1.0f + mana_bar_margin));

		tz::v2f pos = player.mana_bar_pos;
		pos[1] *= (player.good ? 1.0f : -1.0f);
		pos[0] += (manapct * player.mana_bar_dimensions[0]) - bg_scale[0] + yoffset;
		game::render::quad_set_position(player.mana_bar, pos);
	}

	float player_get_mps(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.mana_regen;
	}

	void player_set_mps(player_handle p, float mps)
	{
		auto& player = players[p.peek()];
		player.mana_regen = mps;
	}

	bool player_targets(player_handle p, entity_handle ent)
	{
		auto& player = players[p.peek()];
		return player.target_entity == ent;
	}

	void player_drop_target_entity(player_handle p)
	{
		auto& player = players[p.peek()];
		if(player.target_entity != tz::nullhand && reticule != tz::nullhand)
		{
			destroy_entity(reticule);
			reticule = tz::nullhand;
		}
		player.target_entity = tz::nullhand;
	}

	bool player_owns(player_handle p, entity_handle e)
	{
		if(players[p.peek()].avatar == e) return true;
		entity_handle par = game::entity_get_owner(e);
		while(par != tz::nullhand)
		{
			if(par == players[p.peek()].avatar)
			{
				return true;
			}
			par = game::entity_get_owner(par);
		}
		return false;
	}

	void player_control_entity(player_handle p, entity_handle ent)
	{
		// should we check if player owns it first?
		// yes for now
		if(!player_owns(p, ent))
		{
			return;
		}
		const auto& player = players[p.peek()];
		if(!player.valid)
		{
			return;
		}

		if(player.type == player_type::human && player.target_entity != tz::nullhand)
		{
			game::entity_set_target(ent, player.target_entity);
		}
		else if(player.type == player_type::human && player.target_location.has_value())
		{
			auto prefab = entity_get_prefab(ent);
			// get leeway but ignore leeway coefficient because ranged chars shouldnt just ignore this.
			const float leeway = config_global_speed_multiplier * config_global_leeway_dist;
			if((player.target_location.value() - entity_get_position(ent)).length() > leeway)
			{
				game::entity_set_target_location(ent, player.target_location.value(), true);
			}
		}
		else
		{
			if(entity_get_prefab(ent).combat)
			{
				// todo: find an enemy entity and chase them
				// todo: prioritise an enemy entity if it has recently attacked the enemy's avatar or one of their entities.
				float min_dist = config_default_aggro_range;
				entity_handle nearest_enemy = tz::nullhand;
				game::iterate_entities([player, ent, &min_dist, &nearest_enemy, p](entity_handle other)
				{
					if(entity_is_ambush(other))
					{
						return;
					}
					if(p == human_player && entity_is_ambush(ent))
					{
						// an entity in ambush (that is not owned by a cpu) will not attack automatically
						// only via red reticule.
						return;
					}
					float dst = (entity_get_position(other) - entity_get_position(ent)).length();
					if(dst >= min_dist) {return;}
					if(!entity_get_prefab(other).combat || !entity_get_prefab(other).attackable) {return;}

					if(entity_is_player_aligned(other) != player.good && entity_get_hp(other) > 0)
					{
						min_dist = dst;
						nearest_enemy = other;
					}
				});

				if(nearest_enemy != tz::nullhand)
				{
					entity_set_target(ent, nearest_enemy);
				}
			}
		}
	}

	player_handle try_get_player_that_controls_entity(entity_handle e)
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			player_handle p = static_cast<tz::hanval>(i);
			if(player_owns(p, e))
			{
				return p;
			}
		}
		return tz::nullhand;
	}

	entity_handle player_get_avatar(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.avatar;
	}

	void player_on_death(player_handle p)
	{
		auto& player = players[p.peek()];
		if(player.good)
		{
			living_good_players--;
			if(p == human_player)
			{
				ui_close_all();
				ui_open_defeat_screen();
			}
		}
		else
		{
			living_evil_players--;
			if(living_evil_players == 0)
			{
				real_player_set_level_complete(game::get_current_level().name);
				game::save();
				card c = player.card_pool[std::rand() % player.card_pool.size()];
				ui_close_all();
				ui_open_win_screen(c);
			}
		}
	}

	entity_handle player_get_target(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.target_entity;
	}

	std::optional<tz::v2f> player_get_target_location(player_handle p)
	{
		auto& player = players[p.peek()];
		return player.target_location;
	}

	bool player_try_spend_mana(player_handle p, unsigned int cost)
	{
		auto& player = players[p.peek()];
		if(player.mana < cost)
		{
			return false;
		}
		player_set_mana(p, player.mana - cost);
		return true;
	}

	unsigned int real_player_get_gold()
	{
		tz::lua_execute("_tmp = players.player.gold or 0");
		return tz_must(tz::lua_get_int("_tmp"));
	}

	void real_player_set_gold(unsigned int gold)
	{
		tz::lua_execute(std::format("players.player.gold = {}", gold));
	}

	void real_player_set_level_complete(std::string level_name)
	{
		tz::lua_execute(std::format("if players.player.completed_levels == nil then players.player.completed_levels = {{}} end lvls = players.player.completed_levels; table.insert(players.player.completed_levels, \"{}\")", level_name));
	}

	bool real_player_has_completed_level(std::string level_name)
	{
		auto levels = real_player_get_completed_levels();
		return std::find(levels.begin(), levels.end(), level_name) != levels.end();
	}

	std::vector<std::string> real_player_get_completed_levels()
	{
		std::vector<std::string> ret;

		tz_must(tz::lua_execute("if players.player.completed_levels == nil then players.player.completed_levels = {{}}; _tmp = 0 else _tmp = #players.player.completed_levels end"));
		auto sz = tz_must(tz::lua_get_int("_tmp"));
		for(std::size_t i = 0; i < sz; i++)
		{
			
			tz::lua_execute(std::format("_internal_index = function(arr, idx) return arr[idx] end _tmp = _internal_index(players.player.completed_levels, {})", i + 1));
			std::string name = tz_must(tz::lua_get_string("_tmp"));
			ret.push_back(name);
		}
		return ret;
	}

	void impl_update_single_player(player_handle p, float delta_seconds)
	{
		auto& player = players[p.peek()];

		if(player.avatar != tz::nullhand && entity_get_hp(player.avatar) > 0)
		{
			float last_whole_mana = std::floor(player.mana);
			player.mana += std::clamp(player.mana_regen * delta_seconds, 0.0f, static_cast<float>(player.max_mana));
			if(std::floor(player.mana) > last_whole_mana)
			{
				player_set_mana(p, player.mana);
			}
		}

		impl_update_human_player();
		switch(player.type)
		{
			case player_type::cpu:
				impl_update_cpu_player(p, delta_seconds);
				impl_cpu_try_play_selected_card(p, delta_seconds);
			break;
			case player_type::human:
				impl_human_handle_deck(p);
			break;
		}
	}

	void impl_update_cpu_player(player_handle p, float delta_seconds)
	{
		const auto& player = players[p.peek()];
		if(player.type != player_type::cpu){return;}

		impl_cpu_try_play_selected_card(p, delta_seconds);

		// for now, just try to play index 0 in the deck if we have a card and we have the mana for it
		const std::size_t play_card_id = 0;
		if(game::deck_size(player.deck) > 0)
		{
			auto card = deck_get_card(player.deck, play_card_id);
			unsigned int power = game::get_prefab(card.name).power;
			if(player_try_spend_mana(p, power * config_mana_cost_per_power))
			{
				impl_cpu_select_card_to_play(p, play_card_id);
			}
		}
	}

	void impl_update_human_player()
	{
		if(human_player == tz::nullhand)
		{
			return;
		}

		auto& player = players[human_player.peek()];
		if(tz::os::is_mouse_clicked(tz::os::mouse_button::right))
		{
			tz::v2f click_pos = render::quad_get_position(render::get_cursor());
			constexpr float click_radius = 0.1f;
			float mindist = std::numeric_limits<float>::max();
			entity_handle closest_enemy = tz::nullhand;
			game::iterate_entities([&closest_enemy, &mindist, click_pos, reticule = reticule](entity_handle ent)
			{
				float dist = (click_pos - game::entity_get_position(ent)).length();
				auto prefab = entity_get_prefab(ent);
				if(ent == reticule || !prefab.combat || !prefab.attackable || entity_is_ambush(ent))
				{
					return;
				}
				if(dist < click_radius)
				{
					if(dist < mindist)
					{
						mindist = dist;
						closest_enemy = ent;
					}
				}
			});

			if(closest_enemy != tz::nullhand)
			{
				player.target_entity = closest_enemy;
				player.target_location = std::nullopt;
			}
			else
			{
				player.target_entity = tz::nullhand;
				player.target_location = click_pos;
			}
		}
		impl_update_reticule();

	}

	void impl_update_reticule()
	{
		tz::v2f loc;
		tz::v3f col;

		auto& player = players[human_player.peek()];
		if(player.target_entity != tz::nullhand && game::entity_is_player_aligned(player.target_entity)) {
			player_drop_target_entity(human_player);
		}
		if(player.target_entity != tz::nullhand)
		{
			// targetting an entity
			loc = game::entity_get_position(player.target_entity);
			col = config_player_reticule_colour_on_entity;
		}
		else if(player.target_location.has_value())
		{
			// targetting a location
			loc = player.target_location.value();
			col = config_player_reticule_colour_on_location;
		}
		else
		{
			return;
		}

		// ok we need a reticule and we need it at "loc"
		if(reticule == tz::nullhand)
		{
			reticule = game::create_entity({.prefab_name = "reticule", .player_aligned = player.good, .position = loc});
		}
		else
		{
			game::entity_set_position(reticule, loc);
		}
		game::entity_set_colour_tint(reticule, col);
	}

	void impl_setup_player(player_handle p, game::prefab prefab)
	{
		player_data& player = players[p.peek()];
		tz::v2f pos = player.good ? config_player_avatar_position : config_enemy_avatar_position;
		pos[1] -= (player.good ? living_good_players : living_evil_players) * 0.1f;
		tz::v2f deck_pos = pos; deck_pos[1] += 0.7f;
		if(player.good){deck_pos[1] *= -1.0f;}


		player.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = deck_pos,
			.scale = {0.7f, 0.7f},
			.cards_face_down = !player.good,
			.player_can_play_cards = player.type == player_type::human
		}});

		tz::v2f mana_bar_background_dimensions = player.mana_bar_dimensions;
		mana_bar_background_dimensions[1] *= (1.0f + mana_bar_margin);
		mana_bar_background_dimensions[0] += (mana_bar_background_dimensions[1] - player.mana_bar_dimensions[1]);
		tz::v2f bar_pos = player.mana_bar_pos;
		bar_pos[1] *= (player.good ? 1.0f : -1.0f);
		player.mana_bar_background = game::render::create_quad({.position = bar_pos, .scale = mana_bar_background_dimensions, .layer = 2});
		player.mana_bar = game::render::create_quad({.position = bar_pos, .scale = player.mana_bar_dimensions, .colour = config_mana_bar_colour, .layer = 3});

		pos[0] *= (static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height());

		if(player.avatar != tz::nullhand)
		{
			game::destroy_entity(player.avatar);
		}
		player.avatar = game::create_entity({.prefab_name = prefab.name, .player_aligned = player.good, .position = pos, .scale = config_avatar_scale});
		// player avatar cannot move by default.
		game::entity_set_movement_speed(player.avatar, 0.0f);
		if(player.good)
		{
			game::entity_face_right(player.avatar);
		}
		else
		{
			game::entity_face_left(player.avatar);
		}
		// players cannot spawn with the ambush state (assassin would be a super op avatar)
		game::entity_break_ambush(player.avatar);

		entity_handle aura = game::create_entity({.prefab_name = "aura", .player_aligned = player.good, .position = tz::v2f::zero(), .parent = player.avatar});
		game::entity_set_colour_tint(aura, player.get_colour());
		game::entity_set_layer(aura, -1);
		player_set_mana(p, 0);
	}

	bool impl_ensure_no_human_players()
	{
		return std::none_of(players.begin(), players.end(), [](const player_data& p)->bool
			{
				return p.valid && p.type == player_type::human;
			});
	}

	void impl_human_handle_deck(player_handle p)
	{
		tz_assert(p == human_player, "human player internal logic error");
		auto& player = players[p.peek()];
		for(std::size_t i = 0; i < game::deck_size(player.deck); i++)
		{
			auto deck_size = game::deck_size(player.deck);
			if(player.deck_hold_array.size() != deck_size)
			{
				player.deck_hold_array.resize(deck_size, false);
			}
			if(deck_card_is_held(player.deck, i))
			{
				player.deck_hold_array[i] = true;
				deck_card_hide_tooltip(player.deck, i);
			}
			else
			{
				if(deck_card_is_mouseover(player.deck, i))
				{
					deck_card_display_tooltip(player.deck, i);
				}
				else
				{
					deck_card_hide_tooltip(player.deck, i);
				}
				// not held anymore
				card c = game::deck_get_card(player.deck, i);
				if(player.deck_hold_array[i])
				{
					auto prefab = game::get_prefab(c.name);
					unsigned int cost = prefab.power * config_mana_cost_per_power;
					bool can_play = true;
					if(prefab.require_target_entity_to_play)
					{
						can_play &= player.target_entity != tz::nullhand;
					}
					if(prefab.require_target_location_to_play)
					{
						can_play &= player.target_location.has_value();
					}
					if(can_play)
					{
						can_play = player_try_spend_mana(p, cost);
					}
					if(can_play)
					{
						// but was last frame. i.e we've just let go of it.
						// play it
						entity_handle ent = game::deck_play_card(player.deck, i, true);
						auto player_pos = game::entity_get_position(player.avatar);
						game::entity_set_position(ent, player_pos);
						tz::v2f click_pos = render::quad_get_position(render::get_cursor());
						// so every entity has a leeway amount meaning it wont exactly go to the card play location.
						// so we manually calculate the target position such that it ends up at the card play location.
						tz::v2f dir = (click_pos - player_pos);
						float dst = dir.length();
						if(dst > 0)
						{
							dir /= dir.length();
						}
						const float leeway = config_global_speed_multiplier * game::entity_get_prefab(ent).movement_speed * config_global_leeway_dist * prefab.leeway_coefficient;

						game::entity_set_owner(ent, player.avatar);
						game::entity_set_target_location(ent, player_pos + (dir * (dst + leeway)));
						// this will destroy the card, so fix up our deck hold array
						player.deck_hold_array.erase(player.deck_hold_array.begin() + i);

						// draw a new card?
						if(player.card_pool.size())
						{
							game::deck_add_card(player.deck, player.card_pool[player.card_pool_cursor]);
							player.card_pool_cursor = (player.card_pool_cursor + 1) % player.card_pool.size();	
						}
					}
					else
					{
						// couldnt afford it.
						player.deck_hold_array[i] = false;
						// destroy and re-add.
						game::deck_reset_card_position(player.deck, i);
					}
				}
			}
		}
	}

	void impl_cpu_select_card_to_play(player_handle p, std::size_t id)
	{
		auto& player = players[p.peek()];
		if(player.cpu_play_card_quad != tz::nullhand)
		{
			return;
		}
		player.cpu_play_timer = 0.0f;
		player.cpu_play_card = deck_get_card(player.deck, id);
		// detach the card
		// its probably face down, we want it face up
		render::handle old_card = deck_detach_card(player.deck, id);
		// so we get the old card, save its position, then delete it and spawn the face-up version and use that instead.
		tz::v2f position = render::quad_get_position(old_card);
		tz::v2f scale = render::quad_get_scale(old_card);

		render::destroy_quad(old_card);
		player.cpu_play_card_quad = game::create_card_sprite(player.cpu_play_card, false);
		render::quad_set_position(player.cpu_play_card_quad, position);
		render::quad_set_scale(player.cpu_play_card_quad, scale);	
	}

	bool impl_cpu_try_play_selected_card(player_handle p, float delta_seconds)
	{
		auto& player = players[p.peek()];
		if(player.cpu_play_card_quad != tz::nullhand)
		{
			tz::v2f pos = render::quad_get_position(player.cpu_play_card_quad);
			player.cpu_play_timer += delta_seconds;
			if(player.cpu_play_timer >= config_computer_play_card_turnaround_time_seconds)
			{
				// we're done. play the card.
				entity_handle ent = game::create_entity({.prefab_name = player.cpu_play_card.name, .player_aligned = player.good, .position = player.good ? config_ally_play_position : config_enemy_play_position});
				render::destroy_quad(player.cpu_play_card_quad);

				player.cpu_play_card = {};
				player.cpu_play_card_quad = tz::nullhand;
				player.cpu_play_timer = 0.0f;

				game::entity_set_owner(ent, player.avatar);
				// set it to wander to any location between the avatar and the middle of the screen.
				tz::v2f mid = tz::v2f::zero();
				tz::v2f avatar_pos = game::entity_get_position(player.avatar);
				float dist = (mid - avatar_pos).length();

				tz::v2f midpoint = (mid + avatar_pos) / 2.0f;
				
				tz::v2f target_loc = midpoint;
				target_loc[0] += dist * ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f);
				target_loc[1] += dist * ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f);
				game::entity_set_target_location(ent, target_loc);
				player_control_entity(p, ent);
				
				// draw a new card?
				if(player.card_pool.size())
				{
					game::deck_add_card(player.deck, player.card_pool[player.card_pool_cursor]);
					player.card_pool_cursor = (player.card_pool_cursor + 1) % player.card_pool.size();	
				}
				return true;
			}
			else
			{
				// lerp
				tz::v2f playpos = player.good ? config_ally_play_position : config_enemy_play_position;
				pos[0] = std::lerp(pos[0], playpos[0], delta_seconds * config_computer_play_card_drag_speed);
				pos[1] = std::lerp(pos[1], playpos[1], delta_seconds * config_computer_play_card_drag_speed);
				render::quad_set_position(player.cpu_play_card_quad, pos);
			}
		}
		return false;
	}

	int impl_get_player_prefab_data();
	std::unordered_map<std::string, player_prefab> player_prefab_data;

	void player_setup()
	{
		tz::lua_define_function("callback_player_prefab", impl_get_player_prefab_data);
		tz::lua_execute(R"(
			for k, v in pairs(players) do
				callback_player_prefab(k)
			end
		)");
	}

	player_prefab& get_player_prefab(const std::string& name)
	{
		return player_prefab_data[name];
	}

	// impl bits
	void impl_collect_prefab_deck(std::string_view prefab_name, std::vector<std::string>& deck)
	{
		deck.clear();
		tz::lua_execute(std::format(R"(
		c = players.{}
		has_deck = c.deck ~= nil
		)", prefab_name));
		auto has_deck = tz_must(tz::lua_get_bool("has_deck"));
		if(has_deck)
		{
			tz_must(tz::lua_execute(R"(
			d = c.deck
			_count = #d
			)"));
			int deck_size = tz_must(tz::lua_get_int("_count"));
			deck.reserve(deck_size);
			for(std::size_t i = 0; i < deck_size; i++)
			{
				tz_must(tz::lua_execute(std::format("_tmp = _internal_index(d, {})", i + 1)));
				std::string card_name = tz_must(tz::lua_get_string("_tmp"));
				deck.push_back(card_name);
			}
		}
	}

	template<typename T>
	bool impl_collect_player_prefab_data(std::string_view player_prefab_name, const char* data_name, T& data)
	{
		tz_must(tz::lua_execute(std::format(R"(
			myval = players.{}.{}
			myval_nil = myval == nil
			--if myval == nil then error("\"players.{}.{}\" expected to be non-nil, but it's nil") end
		)", player_prefab_name, data_name, player_prefab_name, data_name)));
		bool is_nil = tz_must(tz::lua_get_bool("myval_nil"));
		if(is_nil)
		{
			return false;
		}
		if constexpr(std::is_same_v<T, bool>)
		{
			data = tz_must(tz::lua_get_bool("myval"));
		}
		else if constexpr(std::numeric_limits<T>::is_integer)
		{
			data = tz_must(tz::lua_get_int("myval"));
		}
		else if constexpr(std::is_floating_point_v<T>)
		{
			data = tz_must(tz::lua_get_number("myval"));
		}
		else if constexpr(std::is_same_v<T, std::string>)
		{
			data = tz_must(tz::lua_get_string("myval"));
		}
		else
		{
			static_assert(false, "woops");
		}
		return true;
	}

	int impl_get_player_prefab_data()
	{
		auto [player_prefab_name] = tz::lua_parse_args<std::string>();
		player_prefab& data = player_prefab_data[player_prefab_name];
		tz::lua_execute(std::format(R"(
			_internal_index = function(arr, idx) return arr[idx] end
			c = player_prefabs.{}
		)", player_prefab_name));
		data.name = player_prefab_name;
		impl_collect_player_prefab_data(player_prefab_name, "avatar", data.avatar_prefab);
		impl_collect_player_prefab_data(player_prefab_name, "mana_coeff", data.mana_coefficient);
		impl_collect_prefab_deck(player_prefab_name, data.deck);
		return 0;
	}

	player_handle load_player_prefab(const player_prefab& prefab, bool cpu, bool player_aligned)
	{
		player_handle ret = game::create_player(cpu ? game::player_type::cpu : game::player_type::human, player_aligned, game::get_prefab(prefab.avatar_prefab), prefab.mana_coefficient);
		std::vector<game::card> cards(prefab.deck.size());
		std::transform(prefab.deck.begin(), prefab.deck.end(), cards.begin(),
			[](const std::string& card)->game::card
			{
				return {.type = game::card_type::creature, .name = card};
			});
		game::player_set_pool(ret, cards);
		auto initial_hand_size = std::min(cards.size(), static_cast<std::size_t>(4));
		for(std::size_t i = 0; i < initial_hand_size; i++)
		{
			auto idx = std::rand() % prefab.deck.size();
			game::deck_add_card(game::player_deck(ret), {.name = prefab.deck[idx]});
		}
		return ret;
	}
}
