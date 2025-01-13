#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

#include "card.hpp"
#include "player.hpp"
#include "prefab.hpp"
#include "entity.hpp"
#include "level.hpp"
#include "render.hpp"
#include "script.hpp"
#include "ui.hpp"
#include "save.hpp"

void render_setup();
void collect_prefab_data();
game::render::handle test_spawn_prefab(const char* prefab_name);

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise({.name = "Super Fun Card Game"});
	tz::os::open_window();
	tz::os::window_fullscreen();
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
	game::render::setup();

	game::script_initialise();
	game::prefab_setup();
	game::card_setup();
	game::level_setup();
	game::load_save();
	game::player_setup();
	game::ui_setup();

	game::ui_open_main_menu();

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	std::uint64_t time = tz::time_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::time_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		if(game::ui_pause_menu_opened())
		{
			delta_seconds = 0;
		}

		game::render::update(delta_seconds);
		game::entity_update(delta_seconds);
		game::player_update(delta_seconds);
		game::deck_update(delta_seconds);
		game::ui_advance();
		tz::os::window_update();
	}
	game::save();
	tz::terminate();
}
