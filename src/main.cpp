#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

#include "card.hpp"
#include "creature.hpp"
#include "entity.hpp"
#include "render.hpp"
#include "script.hpp"

void render_setup();
void collect_creature_data();
game::render::handle test_spawn_creature(const char* creature_name);

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	tz::os::window_fullscreen();
	tz_must(tz::gpu::use_hardware(tz::gpu::find_best_hardware()));
	game::render::setup();

	game::script_initialise();
	game::creature_setup();
	game::card_setup();

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	for(std::size_t i = 0; i < 5; i++)
	{
		game::card card{.type = game::card_type::creature, .name = "peasant"};
		if(i % 2 == 0)
		{
			card.name = "skeleton";
		}
		if(i == 0)
		{
			card.name = "banshee";
		}
		if(i == 1)
		{
			card.name = "knight";
		}
		if(i == 2)
		{
			card.name = "melistra";
		}
		game::render::handle cardsprite = game::create_card_sprite(card);
		game::render::quad_set_position(cardsprite, {i * 0.3f, -0.5f});
	}

	game::entity_handle player = game::create_entity({.creature = "melistra", .position = {-1.2f, 0.0f}, .scale = {-1.5f, 1.5f}});
	game::entity_handle skel = game::create_entity({.creature = "skeleton"});

	std::uint64_t time = tz::system_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::system_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		game::render::update(delta_seconds);
		game::entity_update(delta_seconds);
		tz::os::window_update();
		if(tz::os::is_key_pressed(tz::os::key::w))
		{
			game::entity_start_casting(player);
		}
		if(tz::os::is_key_pressed(tz::os::key::s))
		{
			game::entity_stop_casting(player);
		}
		if(tz::os::is_key_pressed(tz::os::key::escape))
		{
			break;
		}
	}
	tz::terminate();
}