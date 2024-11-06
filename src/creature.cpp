#include "creature.hpp"
#include "tz/core/lua.hpp"
#include <unordered_map>

namespace game
{
	int impl_get_creature_data();

	std::unordered_map<std::string, creature_prefab> creature_data;

	void creature_setup()
	{
		tz::lua_define_function("callback_creature", impl_get_creature_data);
		tz::lua_execute(R"(
			for k, v in pairs(creatures) do
				callback_creature(k)
			end
		)");
	}

	const creature_prefab& get_creature_prefab(const std::string& name)
	{
		return creature_data[name];
	}

	// impl bits
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

	int impl_get_creature_data()
	{
		auto [creature_name] = tz::lua_parse_args<std::string>();
		creature_prefab& data = creature_data[creature_name];
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
}