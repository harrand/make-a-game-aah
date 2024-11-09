#include "render.hpp"
#include "tz/os/input.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/file.hpp"
#include "tz/os/window.hpp"
#include "tz/io/image.hpp"
#include "tz/gpu/settings.hpp"

#include <vector>
#include <unordered_map>

namespace game::render
{
	tz::ren::quad_renderer_handle ren;
	handle background;
	handle cursor;
	std::uint32_t bgimg;
	bool clicked_last_frame = false;

	struct quad_private_data
	{
		quad_flag flags = static_cast<quad_flag>(0);
		flipbook_handle flipbook = tz::nullhand;
		float flipbook_timer = 0.0f;
		bool held = false;
		tz::v2f held_offset = tz::v2f::filled(0.0f);
		bool garbage = false;
	};
	std::vector<quad_private_data> quad_privates = {};

	struct flipbook_data
	{
		unsigned int fps;
		bool repeat;
		std::vector<texture_id> frames = {};
	};
	std::vector<flipbook_data> flipbooks = {};
	struct texture_info
	{
		texture_id texid;
		tz::io::image_header hdr;
		std::vector<std::byte> imgdata;
	};
	std::unordered_map<std::string, texture_info> texture_cache = {};
	std::unordered_map<texture_id, std::string> texture_infos = {};

	void setup()
	{
		tz::gpu::settings_set_vsync(true);
		ren = tz_must(tz::ren::create_quad_renderer
		({
			.clear_colour = {0.3f, 0.3f, 0.3f, 1.0f},
			.flags =
				tz::ren::quad_renderer_flag::graph_present_after
			|	tz::ren::quad_renderer_flag::alpha_clipping
			|	tz::ren::quad_renderer_flag::allow_negative_scale

		}));

		bgimg = create_image_from_file("./res/images/bgforest.png");
		background = create_quad({.scale = tz::v2f::filled(1.0f), .texture_id = bgimg}, quad_flag::match_image_ratio);

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
			handle q = static_cast<tz::hanval>(i);
			auto& quadpriv = quad_privates[i];
			if(quadpriv.garbage)
			{
				continue;
			}
			if(quadpriv.flipbook != tz::nullhand)
			{
				quadpriv.flipbook_timer += delta_seconds;
				const auto& flipbook = flipbooks[quadpriv.flipbook.peek()];

				auto flipbook_time_secs = static_cast<float>(flipbook.frames.size()) / flipbook.fps;
				if(flipbook.repeat && quadpriv.flipbook_timer > flipbook_time_secs)
				{
					quadpriv.flipbook_timer = delta_seconds;
				}
				int flipbook_cursor = (quadpriv.flipbook_timer / flipbook_time_secs) * flipbook.frames.size();
				flipbook_cursor = std::clamp(flipbook_cursor, 0, static_cast<int>(flipbook.frames.size()) - 1);
				quad_set_texture(q, flipbook.frames[flipbook_cursor]);
			}
			if(quadpriv.flags & quad_flag::match_image_ratio)
			{
				std::uint32_t texid = tz::ren::get_quad_texture(ren, q);
				const texture_info& tex = texture_cache[texture_infos[texid]];
				float aspect_ratio = static_cast<float>(tex.hdr.width) / tex.hdr.height;
				tz::v2f scale = tz::ren::get_quad_scale(ren, q);
				scale[0] = scale[1] * aspect_ratio;
				tz::ren::set_quad_scale(ren, q, scale);
			}

			if(quadpriv.flags & quad_flag::draggable)
			{
				tz::v2f pos = tz::ren::get_quad_position(ren, q);
				tz::v2f scale = tz::ren::get_quad_scale(ren, q) * 0.95f;
				tz::v2f min = pos - scale;
				tz::v2f max = pos + scale;

				bool in_region = 
				   min[0] <= mouse_world_pos[0] && mouse_world_pos[0] <= max[0]
				&& min[1] <= mouse_world_pos[1] && mouse_world_pos[1] <= max[1];

				if(!clicked_last_frame && tz::os::is_mouse_clicked(tz::os::mouse_button::left) && in_region)
				{
					// we just started clicking this frame
					// we're holding this now.
					quadpriv.held = true;
					quadpriv.held_offset = pos - mouse_world_pos;
				}
			}
			
			if(quadpriv.held)
			{
				quad_set_position(q, mouse_world_pos + quadpriv.held_offset);
				if(!tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					quadpriv.held = false;
				}
			}
		}
		clicked_last_frame = tz::os::is_mouse_clicked(tz::os::mouse_button::left);
	}

	handle get_cursor()
	{
		return cursor;
	}

	handle get_background()
	{
		return background;
	}

	handle create_quad(tz::ren::quad_info info, quad_flag flags)
	{
		tz::ren::quad_handle ret = tz_must(tz::ren::quad_renderer_create_quad(ren, info));
		if(ret.peek() >= quad_privates.size())
		{
			// not recycled, its a new quad private.
			quad_privates.push_back({.flags = flags});
		}
		else
		{
			quad_privates[ret.peek()] = {.flags = flags};
		}
		return ret;
	}

	void destroy_quad(handle q)
	{
		auto& priv = quad_privates[q.peek()];
		priv = {};
		priv.garbage = true;
		return tz_must(tz::ren::quad_renderer_destroy_quad(ren, q));
	}

	tz::v2f quad_get_position(handle q)
	{
		return tz::ren::get_quad_position(ren, q);
	}

	void quad_set_position(handle q, tz::v2f pos)
	{
		tz::ren::set_quad_position(ren, q, pos);
	}

	tz::v2f quad_get_scale(handle q)
	{
		return tz::ren::get_quad_scale(ren, q);
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
		auto& priv = quad_privates[q.peek()];
		if(priv.flipbook == flipbook)
		{
			return;
		}
		priv.flipbook = flipbook;
		priv.flipbook_timer = 0.0f;
	}

	bool quad_is_held(handle q)
	{
		return quad_privates[q.peek()].held;
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

	std::span<const std::uint32_t> flipbook_get_frames(flipbook_handle flipbook)
	{
		return flipbooks[flipbook.peek()].frames;
	}

	std::uint32_t create_image_from_data(tz::io::image_header imghdr, std::span<const std::byte> imgdata, std::string name)
	{
		texture_id ret = tz_must(tz::ren::quad_renderer_add_texture(ren,
			tz_must(tz::gpu::create_image
			({
				.width = imghdr.width,
				.height = imghdr.height,
				.data = imgdata,
				.name = name.c_str()
			}))
		));
		texture_cache[name] = texture_info{.texid = ret, .hdr = imghdr};
		texture_cache[name].imgdata.resize(imgdata.size());
		std::copy(imgdata.begin(), imgdata.end(), texture_cache[name].imgdata.begin());
		texture_infos[ret] = name;
		return ret;
	}

	std::uint32_t create_image_from_file(std::filesystem::path imgfile)
	{
		// if we already added this image file, don't dupe it, just return the id it was given earlier.
		// this works so long as the GPU doesnt write to these files or bro has photoshop open editing the same file smh.
		auto imgfile_str = imgfile.string();
		if(texture_cache.contains(imgfile_str))
		{
			return texture_cache.at(imgfile_str).texid;
		}

		std::string filedata = tz_must(tz::os::read_file(imgfile));
		tz::io::image_header imghdr = tz_must(tz::io::image_info(tz::view_bytes(filedata)));

		std::vector<std::byte> imgdata(imghdr.data_size_bytes);
		tz_must(tz::io::parse_image(tz::view_bytes(filedata), imgdata));
		return create_image_from_data(imghdr, imgdata, imgfile.string());
	}

	tz::io::image_header get_image_info(std::uint32_t texture_id)
	{
		return texture_cache.at(texture_infos.at(texture_id)).hdr;
	}

	std::span<const std::byte> get_image_data(std::uint32_t texture_id)
	{
		return texture_cache.at(texture_infos.at(texture_id)).imgdata;
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