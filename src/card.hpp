#ifndef GAME_CARD_HPP
#define GAME_CARD_HPP
#include "render.hpp"

namespace game
{
	constexpr short card_layer = 5;
	enum class card_type
	{
		creature
	};

	struct card
	{
		card_type type;
		std::string name;

		bool operator==(const card& rhs) const = default;
	};

	void card_setup();
	render::handle create_card_sprite(card c, bool draggable);	
	render::handle create_card_sprite_facedown(card c, bool draggable);
	std::string card_get_name(card c);
	std::string card_get_description(card c);
}

#endif // GAME_CARD_HPP
