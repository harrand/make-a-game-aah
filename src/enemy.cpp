#include "enemy.hpp"
#include "entity.hpp"

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	struct enemy_data
	{
		entity_handle avatar = tz::nullhand;
		deck_handle deck;
		unsigned int max_mana = 10;
		float mana = 0;
		float mana_regen = 1.0f;

		const tz::v2f mana_bar_pos = {0.0f, 0.8f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
		std::vector<bool> deck_hold_array = {};
	} enemy;

	void enemy_setup(game::prefab prefab)
	{
		enemy.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = {-0.5f, 0.5f},
			.scale = {1.0f, 1.0f}
		}});

		tz::v2f mana_bar_background_dimensions = enemy.mana_bar_dimensions;
		mana_bar_background_dimensions[1] *= (1.0f + mana_bar_margin);
		mana_bar_background_dimensions[0] += (mana_bar_background_dimensions[1] - enemy.mana_bar_dimensions[1]);
		enemy.mana_bar_background = game::render::create_quad({.position = enemy.mana_bar_pos, .scale = mana_bar_background_dimensions, .layer = 2});
		enemy.mana_bar = game::render::create_quad({.position = enemy.mana_bar_pos, .scale = enemy.mana_bar_dimensions, .colour = {0.1f, 0.2f, 0.8f}, .layer = 3});

		enemy_set_creature(prefab);
	}

	void enemy_update(float delta_seconds)
	{
		float last_whole_mana = std::floor(enemy.mana);
		enemy.mana += std::clamp(enemy.mana_regen * delta_seconds, 0.0f, static_cast<float>(enemy.max_mana));
		if(std::floor(enemy.mana) > last_whole_mana)
		{
			enemy_set_mana(enemy.mana);
		}

		for(std::size_t i = 0; i < game::deck_size(enemy.deck); i++)
		{
			auto deck_size = game::deck_size(enemy.deck);
			if(enemy.deck_hold_array.size() != deck_size)
			{
				enemy.deck_hold_array.resize(deck_size, false);
			}
			if(deck_card_is_held(enemy.deck, i))
			{
				enemy.deck_hold_array[i] = true;
			}
			else
			{
				// not held anymore
				card c = game::deck_get_card(enemy.deck, i);
				if(enemy.deck_hold_array[i])
				{
					unsigned int cost = game::get_prefab(c.name).power;
					if(enemy_try_spend_mana(cost))
					{
						// but was last frame. i.e we've just let go of it.
						// play it
						game::deck_play_card(enemy.deck, i, true);
						// this will destroy the card, so fix up our deck hold array
						enemy.deck_hold_array.erase(enemy.deck_hold_array.begin() + i);
					}
					else
					{
						// couldnt afford it.
						enemy.deck_hold_array[i] = false;
						// destroy and re-add.
						game::deck_reset_card_position(enemy.deck, i);
					}
				}
			}
		}
	}

	void enemy_set_creature(game::prefab prefab)
	{
		if(enemy.avatar != tz::nullhand)
		{
			game::destroy_entity(enemy.avatar);
		}
		enemy.avatar = game::create_entity({.prefab_name = prefab.name, .player_aligned = false, .position = {1.0f, 0.0f}});
		game::entity_face_left(enemy.avatar);
		if(prefab.cast != tz::nullhand)
		{
			game::entity_start_casting(enemy.avatar);
		}
	}

	deck_handle enemy_deck()
	{
		return enemy.deck;
	}

	unsigned int enemy_get_max_mana()
	{
		return enemy.max_mana;
	}
	
	void enemy_set_max_mana(unsigned int mana)
	{
		enemy.max_mana = mana;
		enemy_set_mana(enemy.mana);
	}

	unsigned int enemy_get_mana()
	{
		return enemy.mana;
	}

	void enemy_set_mana(unsigned int mana)
	{
		enemy.mana = std::clamp(mana, 0u, enemy.max_mana);

		float manapct = static_cast<float>(enemy.mana) / enemy.max_mana;
		tz::v2f scale = enemy.mana_bar_dimensions;
		scale[0] *= manapct;
		game::render::quad_set_scale(enemy.mana_bar, scale);

		tz::v2f bg_scale = game::render::quad_get_scale(enemy.mana_bar_background);
		float yoffset = bg_scale[1] - (bg_scale[1] / (1.0f + mana_bar_margin));

		tz::v2f pos = enemy.mana_bar_pos;
		pos[0] += (manapct * enemy.mana_bar_dimensions[0]) - bg_scale[0] + yoffset;
		game::render::quad_set_position(enemy.mana_bar, pos);
	}

	bool enemy_try_spend_mana(unsigned int cost)
	{
		if(enemy.mana < cost)
		{
			return false;
		}
		enemy_set_mana(enemy.mana - cost);
		return true;
	}
}
