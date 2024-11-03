#include "render.hpp"
#include "tz/os/input.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/file.hpp"
#include "tz/os/window.hpp"
#include "tz/io/image.hpp"

#include <vector>

namespace game::render
{
	tz::ren::quad_renderer_handle ren;
	tz::ren::quad_handle background;
	tz::ren::quad_handle cursor;
	std::uint32_t bgimg;

	void setup()
	{
		ren = tz_must(tz::ren::create_quad_renderer
		({
			.clear_colour = {0.3f, 0.3f, 0.3f, 1.0f},
			.flags = tz::ren::quad_renderer_flag::alpha_clipping
		}));

		bgimg = create_image_from_file("./res/images/bgforest.png");
		background = create_quad({.scale = tz::v2f{2.0f, 1.0f}, .texture_id = bgimg});

		cursor = create_quad({.scale = tz::v2f::filled(0.02f), .colour = tz::v3f::zero()});
	}

	void update(float delta_seconds)
	{
		tz::ren::quad_renderer_update(ren);
		tz::gpu::execute(tz::ren::quad_renderer_graph(ren));

		auto [mx, my] = tz::os::get_mouse_position();
		tz::v2f mouse_world_pos = screen_to_world(tz::v2u{mx, my});

		tz::ren::set_quad_position(ren, cursor, mouse_world_pos);
	}

	std::uint32_t background_image()
	{
		return bgimg;
	}

	tz::ren::quad_handle create_quad(tz::ren::quad_info info)
	{
		return tz_must(tz::ren::quad_renderer_create_quad(ren, info));
	}

	void quad_set_texture(tz::ren::quad_handle q, std::uint32_t texture)
	{
		tz::ren::set_quad_texture(ren, q, texture);
	}

	std::uint32_t create_image_from_file(std::filesystem::path imgfile)
	{
		std::string filedata = tz_must(tz::os::read_file(imgfile));
		tz::io::image_header imghdr = tz_must(tz::io::image_info(tz::view_bytes(filedata)));

		std::vector<std::byte> imgdata(imghdr.data_size_bytes);
		tz_must(tz::io::parse_image(tz::view_bytes(filedata), imgdata));
		return tz_must(tz::ren::quad_renderer_add_texture(ren,
			tz_must(tz::gpu::create_image
			({
				.width = imghdr.width,
				.height = imghdr.height,
				.data = imgdata,
				.name = imgfile.string().c_str()
			}))
		));
	}

	tz::v2f screen_to_world(tz::v2u screenpos)
	{
		tz::v2f normalised = {static_cast<float>(screenpos[0]) / tz::os::window_get_width(), 1.0f - static_cast<float>(screenpos[1]) / tz::os::window_get_height()};
		normalised *= 2.0f;
		normalised -= tz::v2f::filled(1.0f);
		normalised[0] *= (static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height());
		return normalised;
	}
}