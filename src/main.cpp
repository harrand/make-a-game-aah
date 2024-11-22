#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

#include "card.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "prefab.hpp"
#include "entity.hpp"
#include "render.hpp"
#include "script.hpp"

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
	game::player_setup(game::get_prefab("melistra"));
	game::enemy_setup(game::get_prefab("banshee"));

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	for(std::size_t i = 0; i < 3; i++)
	{
		game::deck_add_card(game::player_deck(), {.name = "banshee"});
		game::deck_add_card(game::player_deck(), {.name = "peasant"});
		game::deck_add_card(game::player_deck(), {.name = "knight"});
	}

	game::deck_add_card(game::enemy_deck(), {.name = "peasant"});
	game::deck_add_card(game::enemy_deck(), {.name = "knight"});
	game::deck_add_card(game::enemy_deck(), {.name = "knight"});
	game::deck_add_card(game::enemy_deck(), {.name = "knight"});
	game::deck_add_card(game::enemy_deck(), {.name = "knight"});

	game::entity_handle skel = game::create_entity({.prefab_name = "skeleton"});
	game::player_set_mana(1);

	std::uint64_t time = tz::time_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::time_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		game::render::update(delta_seconds);
		game::entity_update(delta_seconds);
		game::player_update(delta_seconds);
		game::enemy_update(delta_seconds);
		tz::os::window_update();
		if(tz::os::is_key_pressed(tz::os::key::escape))
		{
			break;
		}
		if(tz::os::is_key_pressed(tz::os::key::z))
		{
			game::deck_swap_cards(game::player_deck(), 0, 1);
		}
		if(tz::os::is_mouse_clicked(tz::os::mouse_button::right))
		{
			auto [mx, my] = tz::os::get_mouse_click_position(tz::os::mouse_button::right);
			
			game::entity_set_target_location(skel, game::render::screen_to_world({mx, my}));
		}
	}
	tz::terminate();
}