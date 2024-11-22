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

		const tz::v2f mana_bar_pos = {0.0f, 0.9f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
		std::vector<bool> deck_hold_array = {};

		float play_timer = 0.0f;
		card play_card = {};
		render::handle play_card_quad = tz::nullhand;
	} enemy;

	constexpr tz::v2f card_play_position = {1.0f, 0.0f};

	void impl_enemy_play_card(std::size_t id);

	void enemy_setup(game::prefab prefab)
	{
		enemy.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = {-0.5f, 0.7f},
			.scale = {0.7f, 0.7f},
			.cards_face_down = true,
			.player_can_play_cards = false,
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

		// test: try to play a card
		// right now logic will just play the first card in its deck as soon as it has the mana for it.
		if(deck_size(enemy.deck) > 0)
		{
			auto card = deck_get_card(enemy.deck, 0);
			unsigned int power = game::get_prefab(card.name).power;
			if(enemy_try_spend_mana(power))
			{
				impl_enemy_play_card(0);
			}
		}

		if(enemy.play_card_quad != tz::nullhand)
		{
			tz::v2f pos = render::quad_get_position(enemy.play_card_quad);
			enemy.play_timer += delta_seconds;
			if(enemy.play_timer >= 2.0f)
			{
				// we're done. play the card.
				game::create_entity({.prefab_name = enemy.play_card.name, .player_aligned = false, .position = card_play_position});
				render::destroy_quad(enemy.play_card_quad);

				enemy.play_card = {};
				enemy.play_card_quad = tz::nullhand;
				enemy.play_timer = 0.0f;
			}
			else
			{
				// lerp
				pos[0] = std::lerp(pos[0], card_play_position[0], delta_seconds * 10);
				pos[1] = std::lerp(pos[1], card_play_position[1], delta_seconds * 10);
				render::quad_set_position(enemy.play_card_quad, pos);
			}
		}
	}

	void enemy_set_creature(game::prefab prefab)
	{
		constexpr tz::v2f pos{1.5f, 0.0f};
		if(enemy.avatar != tz::nullhand)
		{
			game::destroy_entity(enemy.avatar);
		}
		enemy.avatar = game::create_entity({.prefab_name = prefab.name, .player_aligned = false, .position = pos, .scale = tz::v2f::filled(1.25f)});
		game::entity_face_left(enemy.avatar);
		if(prefab.cast != tz::nullhand)
		{
			game::entity_start_casting(enemy.avatar);
		}

		entity_handle aura = game::create_entity({.prefab_name = "aura", .player_aligned = false, .position = pos, .parent = enemy.avatar});
		game::entity_set_colour_tint(aura, {0.5f, 0.0f, 0.0f});
		game::entity_set_layer(aura, -1);
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

	void impl_enemy_play_card(std::size_t id)
	{
		if(enemy.play_card_quad != tz::nullhand)
		{
			return;
		}
		enemy.play_timer = 0.0f;
		enemy.play_card = deck_get_card(enemy.deck, id);
		// detach the card
		// its probably face down, we want it face up
		render::handle old_card = deck_detach_card(enemy.deck, id);
		// so we get the old card, save its position, then delete it and spawn the face-up version and use that instead.
		tz::v2f position = render::quad_get_position(old_card);
		tz::v2f scale = render::quad_get_scale(old_card);

		render::destroy_quad(old_card);
		enemy.play_card_quad = game::create_card_sprite(enemy.play_card, false);
		render::quad_set_position(enemy.play_card_quad, position);
		render::quad_set_scale(enemy.play_card_quad, scale);
	}
}
