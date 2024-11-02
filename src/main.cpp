#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/os/input.hpp"
#include "tz/ren/quad.hpp"

#include "tz/core/memory.hpp"
#include "tz/os/file.hpp"
#include "tz/io/image.hpp"

tz::ren::quad_renderer_handle ren;
std::uint32_t bgimg;
void render_setup();

tz::v2f screen_to_world(tz::v2u screenpos);

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	tz::os::window_fullscreen();
	render_setup();

	tz::ren::quad_handle bgquad = tz_must(tz::ren::quad_renderer_create_quad(ren, {.scale = tz::v2f{2.0f, 1.0f}, .texture_id = bgimg}));

	tz::ren::quad_handle cursor = tz_must(tz::ren::quad_renderer_create_quad(ren, {.scale = tz::v2f{0.02f, 0.02f}, .colour = tz::v3f::zero()}));

	tz::ren::quad_handle quad1 = tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = tz::v2f::zero(), .scale = tz::v2f{0.1f, 0.1f} * 5.0f, .colour = {0.0f, 1.0f, 0.25f}}));

	tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = {-0.5f, -0.5f}, .scale = {0.15f, 0.15f}, .colour = {0.5f, 0.1f, 0.85f}}));

	std::string smilefile = tz_must(tz::os::read_file("./res/images/smile.png"));
	tz::io::image_header smilehdr = tz_must(tz::io::image_info(tz::view_bytes(smilefile)));
	std::vector<std::byte> smile_imgdata(smilehdr.data_size_bytes);

	tz_must(tz::io::parse_image(tz::view_bytes(smilefile), smile_imgdata));

	tz::gpu::resource_handle smile = tz_must(tz::gpu::create_image
	({
		.width = smilehdr.width,
		.height = smilehdr.height,
		.data = smile_imgdata,
		.name = "Smile"
	}));
	std::uint32_t smiletex = tz_must(tz::ren::quad_renderer_add_texture(ren, smile));

	tz::ren::set_quad_texture(ren, quad1, smiletex);

	std::uint64_t time = tz::system_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::system_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		tz::ren::quad_renderer_update(ren);
		tz::os::window_update();
		tz::gpu::execute(tz::ren::quad_renderer_graph(ren));
		tz::v2f pos = tz::ren::get_quad_position(ren, quad1);
		tz::v2f scale = tz::ren::get_quad_scale(ren, quad1);
		if(tz::os::is_key_pressed(tz::os::key::a))
		{
			tz::ren::set_quad_position(ren, quad1, pos += tz::v2f{-1.0f, 0.0f} * delta_seconds);
		}
		if(tz::os::is_key_pressed(tz::os::key::d))
		{
			tz::ren::set_quad_position(ren, quad1, pos += tz::v2f{1.0f, 0.0f} * delta_seconds);
		}
		if(tz::os::is_key_pressed(tz::os::key::w))
		{
			tz::ren::set_quad_position(ren, quad1, pos += tz::v2f{0.0f, 1.0f} * delta_seconds);
		}
		if(tz::os::is_key_pressed(tz::os::key::s))
		{
			tz::ren::set_quad_position(ren, quad1, pos += tz::v2f{0.0f, -1.0f} * delta_seconds);
		}

		if(tz::os::is_key_pressed(tz::os::key::i))
		{
			tz::ren::set_quad_scale(ren, quad1, scale + tz::v2f::filled(delta_seconds * 0.5f));
		}
		if(tz::os::is_key_pressed(tz::os::key::k))
		{
			tz::ren::set_quad_scale(ren, quad1, scale - tz::v2f::filled(delta_seconds * 0.5f));
		}
		if(tz::os::is_key_pressed(tz::os::key::escape))
		{
			break;
		}
		auto [mx, my] = tz::os::get_mouse_position();
		tz::v2f mouse = screen_to_world(tz::v2u{mx, my});
		std::printf("{%.2f, %.2f}                \r", mouse[0], mouse[1]);
		tz::ren::set_quad_position(ren, cursor, mouse);
	}
	tz::terminate();
}

void render_setup()
{
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
	ren = tz_must(tz::ren::create_quad_renderer
	({
		.clear_colour = {0.3f, 0.3f, 0.3f, 1.0f},
		.flags = tz::ren::quad_renderer_flag::alpha_clipping,
	}));

	std::string bgforestfile = tz_must(tz::os::read_file("./res/images/bgforest.png"));
	tz::io::image_header bgforesthdr = tz_must(tz::io::image_info(tz::view_bytes(bgforestfile)));
	std::vector<std::byte> bgforest_imgdata(bgforesthdr.data_size_bytes);

	tz_must(tz::io::parse_image(tz::view_bytes(bgforestfile), bgforest_imgdata));

	tz::gpu::resource_handle bgforest = tz_must(tz::gpu::create_image
	({
		.width = bgforesthdr.width,
		.height = bgforesthdr.height,
		.data = bgforest_imgdata,
		.name = "bgforest"
	}));
	bgimg = tz_must(tz::ren::quad_renderer_add_texture(ren, bgforest));
}

tz::v2f screen_to_world(tz::v2u screenpos)
{
	tz::v2f normalised = {static_cast<float>(screenpos[0]) / tz::os::window_get_width(), 1.0f - static_cast<float>(screenpos[1]) / tz::os::window_get_height()};
	normalised *= 2.0f;
	normalised -= tz::v2f::filled(1.0f);
	normalised[0] *= (static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height());
	return normalised;
}