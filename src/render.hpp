#ifndef GAME_RENDER_HPP
#define GAME_RENDER_HPP
#include "tz/ren/quad.hpp"
#include <filesystem>

namespace game::render
{
	void setup();
	void update(float delta_seconds);
	std::uint32_t background_image();
	tz::ren::quad_handle create_quad(tz::ren::quad_info info);
	void quad_set_texture(tz::ren::quad_handle q, std::uint32_t texture);

	std::uint32_t create_image_from_file(std::filesystem::path imgfile);

	tz::v2f screen_to_world(tz::v2u screenpos);
}

#endif // GAME_RENDER_HPP