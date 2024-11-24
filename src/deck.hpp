#ifndef GAME_DECK_HPP
#define GAME_DECK_HPP
#include "card.hpp"
#include "tz/core/handle.hpp"

namespace game
{
	struct deck_render_info
	{
		tz::v2f position = tz::v2f::zero();
		tz::v2f scale = tz::v2f::filled(1.0f);
		bool cards_face_down = false;
		bool player_can_play_cards = false;
	};

	struct deck_info
	{
		std::span<const card> initial_cards = {};
		std::optional<deck_render_info> sprite = std::nullopt;
	};
	
	using deck_handle = tz::handle<deck_info>;
	deck_handle create_deck(deck_info info = {});
	void destroy_deck(deck_handle deck);
	void deck_update(float delta_seconds);

	void deck_add_card(deck_handle deck, card c);
	void deck_destroy_card(deck_handle deck, std::size_t id);
	void deck_swap_cards(deck_handle deck, std::size_t id1, std::size_t id2);
	void deck_reset_card_position(deck_handle deck, std::size_t id);
	std::size_t deck_size(deck_handle deck);
	card deck_get_card(deck_handle deck, std::size_t id);
	bool deck_card_is_held(deck_handle deck, std::size_t id);
	bool deck_card_is_mouseover(deck_handle deck, std::size_t id);

	render::handle deck_detach_card(deck_handle deck, std::size_t id);

	void deck_card_display_tooltip(deck_handle deck, std::size_t id);
	void deck_card_hide_tooltip(deck_handle deck, std::size_t id);

	void deck_play_card(deck_handle deck, std::size_t id, bool player_aligned);
}

#endif // GAME_DECK_HPP