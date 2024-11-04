#ifndef GAME_RENDER_HPP
#define GAME_RENDER_HPP
#include "tz/ren/quad.hpp"
#include <filesystem>

namespace game::render
{
	void setup();
	void update(float delta_seconds);

	using texture_id = std::uint32_t;
	using handle = tz::ren::quad_handle;
	namespace detail{struct flipbook_t{};}
	using flipbook_handle = tz::handle<detail::flipbook_t>;

	enum quad_flag
	{
		draggable = 0b0001,
		match_image_ratio = 0b0010,
	};
	constexpr quad_flag operator|(quad_flag lhs, quad_flag rhs)
	{
		return static_cast<quad_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(quad_flag lhs, quad_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	handle get_cursor();
	handle get_background();
	handle create_quad(tz::ren::quad_info info, quad_flag flags = static_cast<quad_flag>(0));
	void quad_set_position(handle q, tz::v2f pos);
	void quad_set_scale(handle q, tz::v2f scale);
	void quad_set_colour(handle q, tz::v3f colour);
	void quad_set_texture(handle q, std::uint32_t texture);
	void quad_set_flipbook(handle q, flipbook_handle flipbook);

	flipbook_handle create_flipbook(unsigned int fps, bool repeat);
	void flipbook_add_frame(flipbook_handle flipbook, texture_id tex);

	std::uint32_t create_image_from_file(std::filesystem::path imgfile);

	tz::v2f screen_to_world(tz::v2u screenpos);
}

#endif // GAME_RENDER_HPP