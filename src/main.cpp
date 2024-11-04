#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/settings.hpp"

#include "render.hpp"
#include "script.hpp"

void render_setup();

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	tz::os::window_fullscreen();
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
	tz::gpu::settings_set_vsync(true);
	game::render::setup();

	game::script_initialise();

	tz::ren::quad_handle quad1 = game::render::create_quad({.position = {-1.0f, 0.0f}, .scale = tz::v2f::filled(0.2f), .colour = {0.0f, 1.0f, 0.25f}});
	game::render::quad_set_colour(quad1, {1.0f, 0.0f, 0.0f});

	game::render::flipbook_handle face = game::render::create_flipbook(2, true);
	game::render::flipbook_add_frame(face, game::render::create_image_from_file("./res/images/smile.png"));
	game::render::flipbook_add_frame(face, game::render::create_image_from_file("./res/images/frown.png"));

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(quad1, face);
	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	std::uint64_t time = tz::system_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::system_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		game::render::update(delta_seconds);
		tz::os::window_update();
		if(tz::os::is_key_pressed(tz::os::key::escape))
		{
			break;
		}
	}
	tz::terminate();
}