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
	};

	void card_setup();
	render::handle create_card_sprite(card c);	
}

#endif // GAME_CARD_HPP