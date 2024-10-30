#include "quad_renderer.hpp"

#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"

void render_setup();

int main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	render_setup();
	quad::setup();

	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(quad::renderer());
	}
	tz::terminate();
}

void render_setup()
{
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
}