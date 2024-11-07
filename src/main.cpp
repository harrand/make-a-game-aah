#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

#include "card.hpp"
#include "creature.hpp"
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

	tz::ren::quad_handle quad1 = game::render::create_quad({.position = {-1.0f, 0.0f}, .scale = tz::v2f::filled(0.2f), .colour = {0.0f, 1.0f, 0.25f}}, game::render::quad_flag::draggable);
	game::render::quad_set_colour(quad1, {1.0f, 0.0f, 0.0f});

	game::render::flipbook_handle face = game::render::create_flipbook(2, true);
	game::render::flipbook_add_frame(face, game::render::create_image_from_file("./res/images/smile.png"));
	game::render::flipbook_add_frame(face, game::render::create_image_from_file("./res/images/frown.png"));

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(quad1, face);
	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	for(std::size_t i = 0; i < 4; i++)
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
		game::render::handle cardsprite = game::create_card_sprite(card);
		game::render::quad_set_position(cardsprite, {i * 0.3f, -0.5f});
	}

	test_spawn_creature("peasant");

	auto skel = test_spawn_creature("skeleton");
	game::render::quad_set_position(skel, {0.5f, 0.0f});

	auto banshee = test_spawn_creature("banshee");
	game::render::quad_set_position(banshee, {1.0f, 0.0f});

	auto knight = test_spawn_creature("knight");
	game::render::quad_set_position(knight, {-0.5f, 0.0f});


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

game::render::handle test_spawn_creature(const char* creature_name)
{
	game::creature_prefab prefab = game::get_creature_prefab(creature_name);
	game::render::handle ret = game::render::create_quad({.scale = {0.2f, 0.2f}});
	auto flipbook = prefab.move_horizontal;
	if(flipbook != tz::nullhand)
	{
		game::render::quad_set_flipbook(ret, flipbook);
	}
	return ret;
}