#include "enemy.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "config.hpp"
#include "tz/os/window.hpp"

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	struct enemy_data
	{
		entity_handle avatar = tz::nullhand;
		deck_handle deck;
		unsigned int max_mana = config_default_max_mana;
		float mana = 0;
		float mana_regen = config_default_mps;

		const tz::v2f mana_bar_pos = {0.5f, 0.9f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;

		float play_timer = 0.0f;
		card play_card = {};
		render::handle play_card_quad = tz::nullhand;

		std::vector<card> card_pool = {};
		std::size_t card_pool_cursor = 0;
	} enemy;

	void impl_enemy_play_card(std::size_t id);

	void enemy_setup(game::prefab prefab)
	{
		enemy.deck = game::create_deck({.sprite = game::deck_render_info
		{
			.position = {1.5f, 0.7f},
			.scale = {-0.7f, 0.7f},
			.cards_face_down = true,
			.player_can_play_cards = false,
		}});

		tz::v2f mana_bar_background_dimensions = enemy.mana_bar_dimensions;
		mana_bar_background_dimensions[1] *= (1.0f + mana_bar_margin);
		mana_bar_background_dimensions[0] += (mana_bar_background_dimensions[1] - enemy.mana_bar_dimensions[1]);
		enemy.mana_bar_background = game::render::create_quad({.position = enemy.mana_bar_pos, .scale = mana_bar_background_dimensions, .layer = 2});
		enemy.mana_bar = game::render::create_quad({.position = enemy.mana_bar_pos, .scale = enemy.mana_bar_dimensions, .colour = config_mana_bar_colour, .layer = 3});

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

		// test: try to play a card
		// right now logic will just play the first card in its deck as soon as it has the mana for it.
		if(deck_size(enemy.deck) > 0)
		{
			auto card = deck_get_card(enemy.deck, 0);
			unsigned int power = game::get_prefab(card.name).power;
			if(enemy_try_spend_mana(power * config_mana_cost_per_power))
			{
				impl_enemy_play_card(0);
			}
		}

		if(enemy.play_card_quad != tz::nullhand)
		{
			tz::v2f pos = render::quad_get_position(enemy.play_card_quad);
			enemy.play_timer += delta_seconds;
			if(enemy.play_timer >= config_computer_play_card_turnaround_time_seconds)
			{
				// we're done. play the card.
				entity_handle ent = game::create_entity({.prefab_name = enemy.play_card.name, .player_aligned = false, .position = config_enemy_play_position});
				render::destroy_quad(enemy.play_card_quad);

				enemy.play_card = {};
				enemy.play_card_quad = tz::nullhand;
				enemy.play_timer = 0.0f;

				game::entity_set_target(ent, game::player_get_avatar());
				
				// draw a new card?
				if(enemy.card_pool.size())
				{
					game::deck_add_card(enemy.deck, enemy.card_pool[enemy.card_pool_cursor]);
					enemy.card_pool_cursor = (enemy.card_pool_cursor + 1) % enemy.card_pool.size();	
				}
			}
			else
			{
				// lerp
				pos[0] = std::lerp(pos[0], config_enemy_play_position[0], delta_seconds * config_computer_play_card_drag_speed);
				pos[1] = std::lerp(pos[1], config_enemy_play_position[1], delta_seconds * config_computer_play_card_drag_speed);
				render::quad_set_position(enemy.play_card_quad, pos);
			}
		}
	}

	void enemy_set_creature(game::prefab prefab)
	{
		tz::v2f pos = config_enemy_avatar_position;
		pos[0] *= (static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height());

		if(enemy.avatar != tz::nullhand)
		{
			game::destroy_entity(enemy.avatar);
		}
		enemy.avatar = game::create_entity({.prefab_name = prefab.name, .player_aligned = false, .position = pos, .scale = config_avatar_scale});
		game::entity_face_left(enemy.avatar);
		if(prefab.cast != tz::nullhand)
		{
			game::entity_start_casting(enemy.avatar);
		}

		entity_handle aura = game::create_entity({.prefab_name = "aura", .player_aligned = false, .position = tz::v2f::zero(), .parent = enemy.avatar});
		game::entity_set_colour_tint(aura, config_enemy_aligned_colour);
		game::entity_set_layer(aura, -1);
	}

	deck_handle enemy_deck()
	{
		return enemy.deck;
	}

	void enemy_set_pool(std::span<const card> cards)
	{
		enemy.card_pool.clear();
		enemy.card_pool.resize(cards.size());

		enemy.card_pool_cursor = 0;
		std::copy(cards.begin(), cards.end(), enemy.card_pool.begin());
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

	entity_handle enemy_get_avatar()
	{
		return enemy.avatar;
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
