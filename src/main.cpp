#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/os/input.hpp"
#include "tz/ren/quad.hpp"

tz::ren::quad_renderer_handle ren;
void render_setup();

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	render_setup();

	tz::ren::quad_handle quad1 = tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = tz::v2f::zero(), .scale = tz::v2f{0.1f, 0.1f} * 5.0f, .colour = {0.0f, 1.0f, 0.25f}}));

	tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = {-0.5f, -0.5f}, .scale = {0.15f, 0.15f}, .colour = {0.5f, 0.1f, 0.85f}}));

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
	}
	tz::terminate();
}

void render_setup()
{
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
	ren = tz_must(tz::ren::create_quad_renderer({}));
}