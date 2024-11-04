#include "card.hpp"

namespace game
{
	render::handle create_card_sprite(card c)
	{
		std::uint32_t cardbase_img = render::create_image_from_file("./res/images/cardbase.png");

		return render::create_quad({.scale = tz::v2f::filled(0.2f), .texture_id = cardbase_img, .colour = {1.0f, 0.05f, 0.1f}}, game::render::quad_flag::draggable | game::render::quad_flag::match_image_ratio);
	}
}