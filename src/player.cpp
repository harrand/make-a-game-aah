#include "player.hpp"
#include "card.hpp"

namespace game
{
	constexpr float mana_bar_margin = 0.2f;
	struct player_data
	{
		deck_handle deck;
		unsigned int max_mana = 10;
		float mana = 0;
		float mana_regen = 1.0f;

		const tz::v2f mana_bar_pos = {0.0f, -0.8f};
		const tz::v2f mana_bar_dimensions = {1.0f, 0.04f};
		game::render::handle mana_bar = tz::nullhand;
		game::render::handle mana_bar_background = tz::nullhand;
	} player;

	void player_setup()
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
	}

	void player_update(float delta_seconds)
	{
		float last_whole_mana = std::floor(player.mana);
		player.mana += std::clamp(player.mana_regen * delta_seconds, 0.0f, static_cast<float>(player.max_mana));
		if(std::floor(player.mana) > last_whole_mana)
		{
			player_set_mana(player.mana);
		}
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