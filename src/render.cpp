#include "render.hpp"
#include "tz/os/input.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/file.hpp"
#include "tz/os/window.hpp"
#include "tz/io/image.hpp"
#include "tz/gpu/settings.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedShaderHeader(bloom_blur, vertex)
#include ImportedShaderHeader(bloom_blur, fragment)
#include ImportedShaderHeader(quad_bloom, fragment)

#include <vector>
#include <unordered_map>

namespace game::render
{
	tz::ren::quad_renderer_handle ren;
	tz::gpu::resource_handle output_image = tz::nullhand;
	tz::gpu::resource_handle bloom_image = tz::nullhand;

	tz::gpu::pass_handle bloom_blur_pass = tz::nullhand;
	tz::gpu::graph_handle bloom_blur_graph = tz::nullhand;

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
		bool mouseover = false;
		tz::v2f held_offset = tz::v2f::filled(0.0f);
		bool garbage = false;
	};
	struct quad_extra_shader_data
	{
		std::uint32_t emissive = 0;
	};
	std::vector<quad_private_data> quad_privates = {};

	struct flipbook_data
	{
		unsigned int fps;
		bool repeat;
		std::vector<texture_id> frames = {};
		std::vector<texture_id> emissive_frames = {};
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

	struct glyph_data
	{
		std::unordered_map<char, texture_id> glyphs = {};
	};

	std::unordered_map<std::string, glyph_data> fonts = {};

	struct text_data
	{
		std::vector<handle> character_quads{};
		tz::v2f position;
		tz::v2f scale;
		std::string text;
	};
	std::vector<text_data> texts = {};
	std::vector<text_handle> text_free_list = {};

	void impl_init_fonts();

	void setup()
	{
		// first set graphics settings.
		tz::gpu::settings_set_vsync(true);

		// then setup the bloom blur pass.

		{
			std::vector<std::byte> output_imgdata;
			output_imgdata.resize(tz::os::window_get_width() * tz::os::window_get_height() * 4);
			output_image = tz_must(tz::gpu::create_image
			({
				.type = tz::gpu::image_type::rgba,
				.data = output_imgdata,
				.name = "Game Output Image",
				.flags = tz::gpu::image_flag::colour_target | tz::gpu::image_flag::resize_to_match_window_resource
			}));

			std::vector<std::byte> bloom_imgdata;
			bloom_imgdata.resize(tz::os::window_get_width() * tz::os::window_get_height() * 4);
			bloom_image = tz_must(tz::gpu::create_image
			({
				.type = tz::gpu::image_type::rgba,
				.data = bloom_imgdata,
				.name = "Game Bloom Image",
				.flags = tz::gpu::image_flag::colour_target | tz::gpu::image_flag::resize_to_match_window_resource
			}));

			tz::gpu::resource_handle colour_targets[] =
			{
				tz::gpu::window_resource
			};

			tz::gpu::resource_handle resources[] =
			{
				output_image,
				bloom_image
			};

			bloom_blur_pass = tz_must(tz::gpu::create_pass
			({
				.graphics =
				{
					.colour_targets = colour_targets,
					.culling = tz::gpu::cull::none,
					.flags = tz::gpu::graphics_flag::no_depth_test,
					.triangle_count = 1,
				},
				.shader = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(bloom_blur, vertex), ImportedShaderSource(bloom_blur, fragment))),
				.resources = resources
			}));

			bloom_blur_graph = tz::gpu::create_graph("Bloom Graph");
			tz::gpu::graph_add_pass(bloom_blur_graph, bloom_blur_pass);
		}

		// finally, create the quad renderer.

		{
			tz::gpu::resource_handle colour_targets[] =
			{
				output_image,
				bloom_image
			};

			ren = tz_must(tz::ren::create_quad_renderer
			({
				.clear_colour = {0.3f, 0.3f, 0.3f, 1.0f},
				.colour_targets = colour_targets,
				.post_render = bloom_blur_graph,
				.flags =
					tz::ren::quad_renderer_flag::graph_present_after
				|	tz::ren::quad_renderer_flag::alpha_clipping
				|	tz::ren::quad_renderer_flag::allow_negative_scale
				|	tz::ren::quad_renderer_flag::enable_layering
				|	tz::ren::quad_renderer_flag::custom_fragment_shader,
				.custom_fragment_shader = ImportedShaderSource(quad_bloom, fragment),
				.extra_data_per_quad = sizeof(quad_extra_shader_data),
			}));

			bgimg = create_image_from_file("./res/images/bgbridgeentrance.png");
			background = create_quad({.scale = tz::v2f::filled(1.0f), .texture_id0 = bgimg, .layer = -90}, quad_flag::match_image_ratio);

			cursor = create_quad({.scale = tz::v2f::filled(0.02f), .colour = tz::v3f::zero(), .layer = -85});
		}

		impl_init_fonts();
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
				quad_set_texture0(q, flipbook.frames[flipbook_cursor]);
				quad_set_texture1(q, flipbook.emissive_frames[flipbook_cursor]);
			}
			if(quadpriv.flags & quad_flag::match_image_ratio)
			{
				std::uint32_t texid = tz::ren::get_quad_texture0(ren, q);
				const texture_info& tex = texture_cache[texture_infos[texid]];
				float aspect_ratio = static_cast<float>(tex.hdr.width) / tex.hdr.height;
				tz::v2f scale = tz::ren::get_quad_scale(ren, q);
				scale[0] = scale[1] * aspect_ratio;
				tz::ren::set_quad_scale(ren, q, scale);
			}

			tz::v2f pos = tz::ren::get_quad_position(ren, q);
			tz::v2f scale = tz::ren::get_quad_scale(ren, q) * 0.95f;
			tz::v2f min = pos - scale;
			tz::v2f max = pos + scale;
			if(scale[0] < 0.0f)
			{
				std::swap(min[0], max[0]);
			}
			if(scale[1] < 0.0f)
			{
				std::swap(min[1], max[1]);
			}

			bool in_region = 
				min[0] <= mouse_world_pos[0] && mouse_world_pos[0] <= max[0]
			&& min[1] <= mouse_world_pos[1] && mouse_world_pos[1] <= max[1];

			if(quadpriv.flags & quad_flag::draggable)
			{
				if(!clicked_last_frame && tz::os::is_mouse_clicked(tz::os::mouse_button::left) && in_region)
				{
					// we just started clicking this frame
					// we're holding this now.
					quadpriv.held = true;
					quadpriv.held_offset = pos - mouse_world_pos;
					quad_set_layer(q, quad_get_layer(q) + 1);
				}
			}
			quadpriv.mouseover = in_region;
			
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

		quad_extra_shader_data extra{.emissive = static_cast<bool>((flags & quad_flag::emissive))};
		std::span<const quad_extra_shader_data> extra_span{&extra, 1};
		tz::ren::quad_renderer_set_quad_extra_data(ren, ret, std::as_bytes(extra_span));
		
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

	short quad_get_layer(handle q)
	{
		return tz::ren::get_quad_layer(ren, q);
	}

	void quad_set_layer(handle q, short layer)
	{
		tz::ren::set_quad_layer(ren, q, layer);
	}

	float quad_get_rotation(handle q)
	{
		return tz::ren::get_quad_rotation(ren, q);
	}

	void quad_set_rotation(handle q, float rotation)
	{
		tz::ren::set_quad_rotation(ren, q, rotation);
	}

	tz::v2f quad_get_scale(handle q)
	{
		return tz::ren::get_quad_scale(ren, q);
	}

	void quad_set_scale(handle q, tz::v2f scale)
	{
		tz::ren::set_quad_scale(ren, q, scale);
	}

	tz::v3f quad_get_colour(handle q)
	{
		return tz::ren::get_quad_colour(ren, q);
	}

	void quad_set_colour(handle q, tz::v3f colour)
	{
		tz::ren::set_quad_colour(ren, q, colour);
	}

	void quad_set_texture0(handle q, std::uint32_t texture)
	{
		tz::ren::set_quad_texture0(ren, q, texture);
	}

	void quad_set_texture1(handle q, std::uint32_t texture)
	{
		tz::ren::set_quad_texture1(ren, q, texture);
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
		// instantly set the texture to the first frame of the flipbook, otherwise there will be a frame delay.
		const auto& flipbook_data = flipbooks[flipbook.peek()];
		if(flipbook_data.frames.size())
		{
			quad_set_texture0(q, flipbook_data.frames.front());
		}
	}

	bool quad_is_held(handle q)
	{
		return quad_privates[q.peek()].held;
	}

	bool quad_is_mouseover(handle q)
	{
		return quad_privates[q.peek()].mouseover;
	}

	flipbook_handle create_flipbook(unsigned int fps, bool repeat)
	{
		auto id = flipbooks.size();
		auto& flipbook = flipbooks.emplace_back();
		flipbook.fps = fps;
		flipbook.repeat = repeat;
		return static_cast<tz::hanval>(id);
	}

	void flipbook_add_frame(flipbook_handle flipbookh, texture_id tex, texture_id emissive_tex)
	{
		auto& flipbook = flipbooks[flipbookh.peek()];
		flipbook.frames.push_back(tex);
		flipbook.emissive_frames.push_back(emissive_tex);
	}

	std::span<const std::uint32_t> flipbook_get_frames(flipbook_handle flipbook)
	{
		return flipbooks[flipbook.peek()].frames;
	}

	std::span<const std::uint32_t> flipbook_get_emissive_frames(flipbook_handle flipbook)
	{
		return flipbooks[flipbook.peek()].emissive_frames;
	}

	text_handle create_text(const char* font_name, std::string_view text, tz::v2f position, tz::v2f scale, tz::v3f colour)
	{
		const auto& font = fonts[font_name];

		text_handle ret;
		if(text_free_list.size())
		{
			ret = text_free_list.back();
			text_free_list.pop_back();
		}
		else
		{
			ret = static_cast<tz::hanval>(texts.size());
			texts.push_back({});
		}

		auto& textdata = texts[ret.peek()];
		textdata.position = position;
		textdata.scale = scale;
		textdata.text = text;
		textdata.character_quads.reserve(text.size());
		tz::v2f offset = tz::v2f::zero();
		tz::v2f last_glyph_size;
		for(char c : text)
		{
			texture_id texid = -1u;

			auto iter = font.glyphs.find(c);
			if(iter != font.glyphs.end())
			{
				texid = iter->second;
			}
			else
			{
				// unknown glyph
			}

			tz::v2f pos = position + (offset * scale * 0.0105f);
			if(c == '\n')
			{
				offset[0] = 0.0f;
				offset[1] -= last_glyph_size[1] * 3.0f;
				continue;
			}
			else
			{
				offset[0] += get_image_info(texid).width;
			}
			textdata.character_quads.push_back(create_quad
			({
				.position = pos,
				.scale = scale,
				.texture_id0 = texid,
				.colour = colour,
				.layer = 95
			}));
			const auto& info = get_image_info(texid);
			last_glyph_size = tz::v2u{info.width, info.height};
		}
		return ret;
	}

	void destroy_text(text_handle q)
	{
		auto& text = texts[q.peek()];
		for(handle q : text.character_quads)
		{
			if(q != tz::nullhand)
			{
				destroy_quad(q);
			}
		}
		text.character_quads.clear();
		text_free_list.push_back(q);
	}

	void text_set_position(text_handle q, tz::v2f position)
	{
		auto& text = texts[q.peek()];
		tz::v2f old_position = text.position;
		tz::v2f delta = position - old_position;
		for(render::handle quad : text.character_quads)
		{
			tz::v2f old = game::render::quad_get_position(quad);
			old += delta;
			game::render::quad_set_position(quad, old);
		}
		text.position = position;
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

	void impl_init_fonts()
	{
		for(const auto& entry : std::filesystem::directory_iterator("./res/images/font"))
		{
			if(entry.is_directory())
			{
				std::string font_name = entry.path().filename().string();

				auto& glyphs = fonts[font_name];

				for(const auto& glyph_file : std::filesystem::directory_iterator(entry.path()))
				{
					if(glyph_file.path().has_extension() && glyph_file.path().extension() == ".png")
					{
						char glyph = std::stoi(glyph_file.path().stem().string());
						glyphs.glyphs[glyph] = create_image_from_file(glyph_file.path());
					}
				}
			}
		}
	}
}
