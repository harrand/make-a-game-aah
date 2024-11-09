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
	};

	struct deck_info
	{
		std::span<const card> initial_cards = {};
		std::optional<deck_render_info> sprite = std::nullopt;
	};
	
	using deck_handle = tz::handle<deck_info>;
	deck_handle create_deck(deck_info info = {});
	void destroy_deck(deck_handle deck);

	void deck_add_card(deck_handle deck, card c);
}

#endif // GAME_DECK_HPP