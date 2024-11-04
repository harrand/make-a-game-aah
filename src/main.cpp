#include "tz/topaz.hpp"
#include "tz/core/time.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

#include "tz/core/lua.hpp"

#include "card.hpp"
#include "render.hpp"
#include "script.hpp"

#include <unordered_map>

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

	for(std::size_t i = 0; i < 7; i++)
	{
		game::render::handle cardsprite = game::create_card_sprite(game::card{});
		game::render::quad_set_position(cardsprite, {i * 0.2f, -0.5f});
		if(i == 3)
		{
			game::render::destroy_quad(cardsprite);
		}
	}

	collect_creature_data();
	test_spawn_creature("peasant");

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

struct creature_data_t
{
	std::string name;
	game::render::flipbook_handle idle = tz::nullhand;
};
std::unordered_map<std::string, creature_data_t> creature_data;

int get_creature_data()
{
	auto [creature_name] = tz::lua_parse_args<std::string>();
	creature_data_t& data = creature_data[creature_name];
	tz::lua_execute(std::format(R"(
		_internal_index = function(arr, idx) return arr[idx] end
		c = creatures.{}
		has_name = c.name ~= nil
		has_idle = c.idle ~= nil
		has_max_hp = c.max_hp ~= nil
	)", creature_name));
	auto has_name = tz_must(tz::lua_get_bool("has_name"));
	auto has_idle = tz_must(tz::lua_get_bool("has_idle"));
	if(has_idle)
	{
		tz::lua_execute("_count = #c.idle.frames");
		int frame_count = tz_must(tz::lua_get_int("_count"));
		int fps = tz_must(tz::lua_get_int("c.idle.fps"));
		bool repeats = tz_must(tz::lua_get_bool("c.idle.repeats"));

		data.idle = game::render::create_flipbook(fps, repeats);

		for(std::size_t i = 0; i < frame_count; i++)
		{
			tz::lua_execute(std::format("_tmp = _internal_index(c.idle.frames, {})", i + 1));
			std::string path = std::format("./res/images/{}", tz_must(tz::lua_get_string("_tmp")));
			game::render::flipbook_add_frame(data.idle, game::render::create_image_from_file(path));
		}
	}
	auto has_max_hp = tz_must(tz::lua_get_bool("has_max_hp"));
	return 0;
}

void collect_creature_data()
{
	tz::lua_define_function("callback_creature", get_creature_data);
	tz::lua_execute(R"(
		for k, v in pairs(creatures) do
			callback_creature(k)
		end
	)");
}

game::render::handle test_spawn_creature(const char* creature_name)
{
	game::render::handle ret = game::render::create_quad({.scale = tz::v2f::filled(0.2f)});
	auto flipbook = creature_data[creature_name].idle;
	if(flipbook != tz::nullhand)
	{
		game::render::quad_set_flipbook(ret, flipbook);
	}
	return ret;
}