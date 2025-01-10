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
	game::player_setup();

	game::load_level(game::get_level("forest"));

	auto player = game::create_player(game::player_type::human, true, game::get_prefab("melistra"));

	game::render::flipbook_handle hourglass = game::render::create_flipbook(3, true);
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassv.png"));
	game::render::flipbook_add_frame(hourglass, game::render::create_image_from_file("./res/images/hourglassh.png"));

	game::render::quad_set_flipbook(game::render::get_cursor(), hourglass);

	game::deck_add_card(game::player_deck(player), {.name = "melistra"});
	game::deck_add_card(game::player_deck(player), {.name = "marksman"});
	game::deck_add_card(game::player_deck(player), {.name = "lich"});
	game::deck_add_card(game::player_deck(player), {.name = "warbear"});
	game::deck_add_card(game::player_deck(player), {.name = "undead_bear"});
	game::deck_add_card(game::player_deck(player), {.name = "bear"});
	game::deck_add_card(game::player_deck(player), {.name = "peasant"});
	game::deck_add_card(game::player_deck(player), {.name = "skeletal_warrior"});

	game::card player_cards[] =
	{
		game::card
		{
			.type = game::card_type::creature,
			.name = "peasant"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "knight"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "archer"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "archer"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "assassin"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "knight"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "melistra"
		},
		game::card
		{
			.type = game::card_type::creature,
			.name = "general"
		},
	};
	game::player_set_pool(player, player_cards);

	std::uint64_t time = tz::time_nanos();
	while(tz::os::window_is_open())
	{
		std::uint64_t now = tz::time_nanos();
		float delta_seconds = (now - time) / 1000000000.0f;
		time = now;

		game::render::update(delta_seconds);
		game::entity_update(delta_seconds);
		game::player_update(delta_seconds);
		game::deck_update(delta_seconds);
		tz::os::window_update();
		if(tz::os::is_key_pressed(tz::os::key::escape))
		{
			break;
		}
		if(tz::os::is_key_pressed(tz::os::key::z))
		{
			game::deck_swap_cards(game::player_deck(player), 0, 1);
		}
	}
	tz::terminate();
}
