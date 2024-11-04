#include "render.hpp"
#include "tz/os/input.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/file.hpp"
#include "tz/os/window.hpp"
#include "tz/io/image.hpp"

#include <vector>
#include <unordered_map>

namespace game::render
{
	tz::ren::quad_renderer_handle ren;
	handle background;
	handle cursor;
	std::uint32_t bgimg;

	struct quad_private_data
	{
		flipbook_handle flipbook = tz::nullhand;
		float flipbook_timer = 0.0f;
	};
	std::vector<quad_private_data> quad_privates = {};

	struct flipbook_data
	{
		unsigned int fps;
		bool repeat;
		std::vector<texture_id> frames = {};
	};
	std::vector<flipbook_data> flipbooks = {};
	std::unordered_map<std::filesystem::path, texture_id> texture_cache = {};

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

		for(std::size_t i = 0; i < quad_privates.size(); i++)
		{
			auto& quadpriv = quad_privates[i];
			if(quadpriv.flipbook != tz::nullhand)
			{
				quadpriv.flipbook_timer += delta_seconds;
				const auto& flipbook = flipbooks[quadpriv.flipbook.peek()];

				auto flipbook_time_secs = static_cast<float>(flipbook.frames.size()) / flipbook.fps;
				if(flipbook.repeat && quadpriv.flipbook_timer > flipbook_time_secs)
				{
					quadpriv.flipbook_timer -= flipbook_time_secs;
				}
				int flipbook_cursor = (quadpriv.flipbook_timer / flipbook_time_secs) * flipbook.frames.size();
				flipbook_cursor = std::clamp(flipbook_cursor, 0, static_cast<int>(flipbook.frames.size()) - 1);
				quad_set_texture(static_cast<tz::hanval>(i), flipbook.frames[flipbook_cursor]);
			}
		}
	}

	handle get_cursor()
	{
		return cursor;
	}

	handle get_background()
	{
		return background;
	}

	handle create_quad(tz::ren::quad_info info)
	{
		quad_privates.push_back({});
		return tz_must(tz::ren::quad_renderer_create_quad(ren, info));
	}

	void quad_set_position(handle q, tz::v2f pos)
	{
		tz::ren::set_quad_position(ren, q, pos);
	}

	void quad_set_scale(handle q, tz::v2f scale)
	{
		tz::ren::set_quad_scale(ren, q, scale);
	}

	void quad_set_colour(handle q, tz::v3f colour)
	{
		tz::ren::set_quad_colour(ren, q, colour);
	}

	void quad_set_texture(handle q, std::uint32_t texture)
	{
		tz::ren::set_quad_texture(ren, q, texture);
	}

	void quad_set_flipbook(handle q, flipbook_handle flipbook)
	{
		quad_privates[q.peek()].flipbook = flipbook;
	}

	flipbook_handle create_flipbook(unsigned int fps, bool repeat)
	{
		auto id = flipbooks.size();
		auto& flipbook = flipbooks.emplace_back();
		flipbook.fps = fps;
		flipbook.repeat = repeat;
		return static_cast<tz::hanval>(id);
	}

	void flipbook_add_frame(flipbook_handle flipbookh, texture_id tex)
	{
		auto& flipbook = flipbooks[flipbookh.peek()];
		flipbook.frames.push_back(tex);
	}

	std::uint32_t create_image_from_file(std::filesystem::path imgfile)
	{
		// if we already added this image file, don't dupe it, just return the id it was given earlier.
		// this works so long as the GPU doesnt write to these files or bro has photoshop open editing the same file smh.
		if(texture_cache.contains(imgfile))
		{
			return texture_cache.at(imgfile);
		}

		std::string filedata = tz_must(tz::os::read_file(imgfile));
		tz::io::image_header imghdr = tz_must(tz::io::image_info(tz::view_bytes(filedata)));

		std::vector<std::byte> imgdata(imghdr.data_size_bytes);
		tz_must(tz::io::parse_image(tz::view_bytes(filedata), imgdata));
		texture_id ret = tz_must(tz::ren::quad_renderer_add_texture(ren,
			tz_must(tz::gpu::create_image
			({
				.width = imghdr.width,
				.height = imghdr.height,
				.data = imgdata,
				.name = imgfile.string().c_str()
			}))
		));
		texture_cache[imgfile] = ret;
		return ret;
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