#include "player.hpp"
#include "entity.hpp"

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	struct player_data
	{
		entity_handle avatar = tz::nullhand;
		deck_handle deck;
		unsigned int max_mana = 10;
		float mana = 0;
		float mana_regen = 1.0f;

		const tz::v2f mana_bar_pos = {0.0f, -0.8f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
		std::vector<bool> deck_hold_array = {};
	} player;

	void player_setup(game::prefab prefab)
	{
		player.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = {-0.5f, -0.5f},
			.scale = {1.0f, 1.0f}
		}});

		tz::v2f mana_bar_background_dimensions = player.mana_bar_dimensions;
		mana_bar_background_dimensions[1] *= (1.0f + mana_bar_margin);
		mana_bar_background_dimensions[0] += (mana_bar_background_dimensions[1] - player.mana_bar_dimensions[1]);
		player.mana_bar_background = game::render::create_quad({.position = player.mana_bar_pos, .scale = mana_bar_background_dimensions});
		player.mana_bar = game::render::create_quad({.position = player.mana_bar_pos, .scale = player.mana_bar_dimensions, .colour = {0.1f, 0.2f, 0.8f}});

		player_set_creature(prefab);
	}

	void player_update(float delta_seconds)
	{
		float last_whole_mana = std::floor(player.mana);
		player.mana += std::clamp(player.mana_regen * delta_seconds, 0.0f, static_cast<float>(player.max_mana));
		if(std::floor(player.mana) > last_whole_mana)
		{
			player_set_mana(player.mana);
		}

		auto deck_size = game::deck_size(player.deck);
		if(player.deck_hold_array.size() != deck_size)
		{
			player.deck_hold_array.resize(deck_size, false);
		}
		for(std::size_t i = 0; i < game::deck_size(player.deck); i++)
		{
			if(deck_card_is_held(player.deck, i))
			{
				player.deck_hold_array[i] = true;
			}
			else
			{
				// not held anymore
				if(player.deck_hold_array[i])
				{
					// but was last frame. i.e we've just let go of it.
					// play it
					game::deck_play_card(player.deck, i);
					// this will destroy the card, so fix up our deck hold array
					player.deck_hold_array.erase(player.deck_hold_array.begin() + i);
				}
			}
		}
	}

	void player_set_creature(game::prefab prefab)
	{
		if(player.avatar != tz::nullhand)
		{
			game::destroy_entity(player.avatar);
		}
		player.avatar = game::create_entity({.prefab_name = prefab.name, .position = {-1.0f, 0.0f}});
		// face right
		game::entity_move(player.avatar, {1.0f, 0.0f});
	}

	deck_handle player_deck()
	{
		return player.deck;
	}

	unsigned int player_get_max_mana()
	{
		return player.max_mana;
	}
	
	void player_set_max_mana(unsigned int mana)
	{
		player.max_mana = mana;
		player_set_mana(player.mana);
	}

	unsigned int player_get_mana()
	{
		return player.mana;
	}

	void player_set_mana(unsigned int mana)
	{
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
}