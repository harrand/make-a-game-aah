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
	game::render::flipbook_handle idle = tz::nullhand;
	game::render::flipbook_handle move_horizontal = tz::nullhand;
	game::render::flipbook_handle move_up = tz::nullhand;
	game::render::flipbook_handle move_down = tz::nullhand;
};

std::unordered_map<std::string, creature_data_t> creature_data;

void impl_collect_creature_animation(std::string_view creature_name, const char* animation_name, game::render::flipbook_handle& flipbook)
{
	tz::lua_execute(std::format(R"(
	c = creatures.{}
	has_anim = c.{} ~= nil
	)", creature_name, animation_name));
	auto has_anim = tz_must(tz::lua_get_bool("has_anim"));
	if(has_anim)
	{
		tz::lua_execute(std::format(R"(
		anim = c.{}
		_count = #anim.frames
		)", animation_name));
		int frame_count = tz_must(tz::lua_get_int("_count"));
		int fps = tz_must(tz::lua_get_int("anim.fps"));
		bool loop = tz_must(tz::lua_get_bool("anim.loop"));

		flipbook = game::render::create_flipbook(fps, loop);
		for(std::size_t i = 0; i < frame_count; i++)
		{
			tz::lua_execute(std::format("_tmp = _internal_index(anim.frames, {})", i + 1));
			std::string path = std::format("./res/images/{}", tz_must(tz::lua_get_string("_tmp")));
			game::render::flipbook_add_frame(flipbook, game::render::create_image_from_file(path));
		}
	}
}

int get_creature_data()
{
	auto [creature_name] = tz::lua_parse_args<std::string>();
	creature_data_t& data = creature_data[creature_name];
	tz::lua_execute(std::format(R"(
		_internal_index = function(arr, idx) return arr[idx] end
		c = creatures.{}
	)", creature_name));
	impl_collect_creature_animation(creature_name, "idle", data.idle);
	impl_collect_creature_animation(creature_name, "move_horizontal", data.move_horizontal);
	impl_collect_creature_animation(creature_name, "move_up", data.move_up);
	impl_collect_creature_animation(creature_name, "move_down", data.move_down);
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
	game::render::handle ret = game::render::create_quad({.scale = {0.2f, 0.2f}});
	auto flipbook = creature_data[creature_name].move_horizontal;
	if(flipbook != tz::nullhand)
	{
		game::render::quad_set_flipbook(ret, flipbook);
	}
	return ret;
}