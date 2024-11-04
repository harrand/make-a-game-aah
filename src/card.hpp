#ifndef GAME_CARD_HPP
#define GAME_CARD_HPP
#include "render.hpp"

namespace game
{
	enum class card_type
	{
		creature
	};

	struct card
	{
		card_type type;
		unsigned int id;
	};

	render::handle create_card_sprite(card c);	
}

#endif // GAME_CARD_HPP