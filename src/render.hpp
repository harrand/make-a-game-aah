#ifndef GAME_RENDER_HPP
#define GAME_RENDER_HPP
#include "tz/ren/quad.hpp"
#include "tz/io/image.hpp"
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
	void destroy_quad(handle q);
	tz::v2f quad_get_position(handle q);
	void quad_set_position(handle q, tz::v2f pos);
	short quad_get_layer(handle q);
	void quad_set_layer(handle q, short layer);
	float quad_get_rotation(handle q);
	void quad_set_rotation(handle q, float rotation);
	tz::v2f quad_get_scale(handle q);
	void quad_set_scale(handle q, tz::v2f scale);
	tz::v3f quad_get_colour(handle q);
	void quad_set_colour(handle q, tz::v3f colour);
	void quad_set_texture(handle q, std::uint32_t texture);
	void quad_set_flipbook(handle q, flipbook_handle flipbook);

	bool quad_is_held(handle q);
	bool quad_is_mouseover(handle q);

	flipbook_handle create_flipbook(unsigned int fps, bool repeat);
	void flipbook_add_frame(flipbook_handle flipbook, texture_id tex);

	std::span<const std::uint32_t> flipbook_get_frames(flipbook_handle flipbook);

	namespace detail{struct text_t{};}
	using text_handle = tz::handle<detail::text_t>;
	text_handle create_text(const char* font_name, std::string_view text, tz::v2f position, tz::v2f scale = tz::v2f::filled(1.0f), tz::v3f colour = tz::v3f::filled(1.0f));
	void destroy_text(text_handle q);

	void text_set_position(text_handle q, tz::v2f position);

	std::uint32_t create_image_from_data(tz::io::image_header hdr, std::span<const std::byte> imgdata, std::string name);
	std::uint32_t create_image_from_file(std::filesystem::path imgfile);
	tz::io::image_header get_image_info(std::uint32_t texture_id);
	std::span<const std::byte> get_image_data(std::uint32_t texture_id);

	tz::v2f screen_to_world(tz::v2u screenpos);
}

#endif // GAME_RENDER_HPP