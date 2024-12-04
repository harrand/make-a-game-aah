#include "player.hpp"
#include "config.hpp"
#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include <algorithm>

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	entity_handle reticule = tz::nullhand;

	struct player_data
	{
		player_type type;
		bool good;
		entity_handle avatar = tz::nullhand;
		deck_handle deck;
		unsigned int max_mana = config_default_max_mana;
		float mana = 0;
		float mana_regen = config_default_mps;

		const tz::v2f mana_bar_pos = {-0.5f, -0.9f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
		std::vector<bool> deck_hold_array = {};

		entity_handle target_entity = tz::nullhand;
		std::optional<tz::v2f> target_location = std::nullopt;

		std::vector<card> card_pool = {};
		std::size_t card_pool_cursor = 0;
	};

	std::vector<player_data> players = {};
	player_handle human_player = tz::nullhand;

	void impl_update_reticule();
	void impl_setup_player(player_handle p, game::prefab prefab);
	bool impl_ensure_no_human_players();

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

	player_handle create_player(player_type type, bool good, game::prefab prefab)
	{
		player_handle ret = static_cast<tz::hanval>(players.size());

		if(type == player_type::human)
		{
			tz_assert(impl_ensure_no_human_players(), "cannot add more than one human player");
			human_player = ret;
		}


		players.push_back({.type = type, .good = good});
		impl_setup_player(ret, prefab);

		return ret;
	}


	void player_update(float delta_seconds)
	{
		for(std::size_t i = 0; i < players.size(); i++)
		{
			player_data& player = players[i];
			player_handle p = static_cast<tz::hanval>(i);

			float last_whole_mana = std::floor(player.mana);
			player.mana += std::clamp(player.mana_regen * delta_seconds, 0.0f, static_cast<float>(player.max_mana));
			if(std::floor(player.mana) > last_whole_mana)
			{
				player_set_mana(p, player.mana);
			}

			if(tz::os::is_mouse_clicked(tz::os::mouse_button::right))
			{
				tz::v2f click_pos = render::quad_get_position(render::get_cursor());
				constexpr float click_radius = 0.1f;
				float mindist = std::numeric_limits<float>::max();
				entity_handle closest_enemy = tz::nullhand;
				game::iterate_entities([&closest_enemy, &mindist, click_pos, reticule = reticule](entity_handle ent)
				{
					float dist = (click_pos - game::entity_get_position(ent)).length();
					if(ent == reticule)
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

							if(player.target_entity != tz::nullhand)
							{
								game::entity_set_target(ent, player.target_entity);
							}
							else if(player.target_location.has_value())
							{
								game::entity_set_target_location(ent, player.target_location.value());
							}
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
			float x = static_cast<float>(tz::os::window_get_width()) * 0.9f / tz::os::window_get_height();
			game::render::create_text("kongtext", "YOU DIED :(", {-x, 0.0f}, tz::v2f::filled(0.3f), {1.0f, 0.0f, 0.0f});
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
		tz::v2f deck_pos = pos; deck_pos[1] += 0.7f;
		if(player.good){deck_pos[1] *= -1.0f;}

		player.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = deck_pos,
			.scale = {0.7f, 0.7f},
			.player_can_play_cards = player.good
		}});

		tz::v2f mana_bar_background_dimensions = player.mana_bar_dimensions;
		mana_bar_background_dimensions[1] *= (1.0f + mana_bar_margin);
		mana_bar_background_dimensions[0] += (mana_bar_background_dimensions[1] - player.mana_bar_dimensions[1]);
		player.mana_bar_background = game::render::create_quad({.position = player.mana_bar_pos, .scale = mana_bar_background_dimensions, .layer = 2});
		player.mana_bar = game::render::create_quad({.position = player.mana_bar_pos, .scale = player.mana_bar_dimensions, .colour = config_mana_bar_colour, .layer = 3});

		pos[0] *= (static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height());

		if(player.avatar != tz::nullhand)
		{
			game::destroy_entity(player.avatar);
		}
		player.avatar = game::create_entity({.prefab_name = prefab.name, .player_aligned = player.good, .position = pos, .scale = config_avatar_scale});
		if(player.good)
		{
			game::entity_face_right(player.avatar);
		}
		else
		{
			game::entity_face_left(player.avatar);
		}
		if(prefab.cast != tz::nullhand)
		{
			game::entity_start_casting(player.avatar);
		}

		entity_handle aura = game::create_entity({.prefab_name = "aura", .player_aligned = player.good, .position = tz::v2f::zero(), .parent = player.avatar});
		game::entity_set_colour_tint(aura, player.good ? config_player_aligned_colour : config_enemy_aligned_colour);
		game::entity_set_layer(aura, -1);
	}

	bool impl_ensure_no_human_players()
	{
		return std::none_of(players.begin(), players.end(), [](const player_data& p)->bool
			{
				return p.type == player_type::human;
			});
	}
}
