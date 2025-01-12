#include "level.hpp"
#include "render.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "tz/core/lua.hpp"
#include <unordered_map>

namespace game
{
	int impl_get_level_data();
	std::unordered_map<std::string, level> level_data;
	std::string loaded_level = "";

	void level_setup()
	{
		tz::lua_define_function("callback_level", impl_get_level_data);
		tz::lua_execute(R"(
			for k, v in pairs(levels) do
				callback_level(k)
			end
		)");
	}

	const level& get_level(const std::string& name)
	{
		return level_data[name];
	}

	void iterate_levels(std::function<void(std::string_view, const level&)> callback)
	{
		for(const auto& [name, data] : level_data)
		{
			callback(name, data);
		}
	}

	template<typename T>
	bool impl_collect_level_data(std::string_view level_name, const char* data_name, T& data)
	{
		tz_must(tz::lua_execute(std::format(R"(
			myval = levels.{}.{}
			myval_nil = myval == nil
			--if myval == nil then error("\"levels.{}.{}\" expected to be non-nil, but it's nil") end
		)", level_name, data_name, level_name, data_name)));
		bool is_nil = tz_must(tz::lua_get_bool("myval_nil"));
		if(is_nil)
		{
			return false;
		}
		if constexpr(std::is_same_v<T, bool>)
		{
			data = tz_must(tz::lua_get_bool("myval"));
		}
		else if constexpr(std::numeric_limits<T>::is_integer)
		{
			data = tz_must(tz::lua_get_int("myval"));
		}
		else if constexpr(std::is_floating_point_v<T>)
		{
			data = tz_must(tz::lua_get_number("myval"));
		}
		else if constexpr(std::is_same_v<T, std::string>)
		{
			data = tz_must(tz::lua_get_string("myval"));
		}
		else
		{
			static_assert(false, "woops");
		}
		return true;
	}

	int impl_get_level_data()
	{
		auto [level_name] = tz::lua_parse_args<std::string>();
		level& data = level_data[level_name];
		tz::lua_execute(std::format(R"(
			_internal_index = function(arr, idx) return arr[idx] end
			c = levels.{}
		)", level_name));
		data.name = level_name;
		impl_collect_level_data(level_name, "display_name", data.display_name);
		impl_collect_level_data(level_name, "description", data.description);
		std::string background_image_path;
		impl_collect_level_data(level_name, "background_image", background_image_path);
		impl_collect_level_data(level_name, "player_prefab", data.player_prefab);
		data.background_image = render::create_image_from_file(background_image_path);
		return 0;
	}

	void load_level(const level& l)
	{
		loaded_level = l.name;
		clear_entities();
		render::quad_set_texture0(render::get_background(), l.background_image);
		if(l.player_prefab.size())
		{
			game::load_player_prefab(game::get_player_prefab(l.player_prefab));
		}
	}

	void reload_level()
	{
		load_level(get_level(loaded_level));
	}

}
