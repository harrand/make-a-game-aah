#include "prefab.hpp"
#include "tz/core/lua.hpp"
#include <unordered_map>
#include <limits>

namespace game
{
	int impl_get_prefab_data();

	std::unordered_map<std::string, prefab> prefab_data;

	void prefab_setup()
	{
		tz::lua_define_function("callback_prefab", impl_get_prefab_data);
		tz::lua_execute(R"(
			for k, v in pairs(prefabs) do
				callback_prefab(k)
			end
		)");
	}

	const prefab& get_prefab(const std::string& name)
	{
		return prefab_data[name];
	}

	// impl bits
	void impl_collect_prefab_animation(std::string_view prefab_name, const char* animation_name, game::render::flipbook_handle& flipbook)
	{
		tz::lua_execute(std::format(R"(
		c = prefabs.{}
		has_anim = c.{} ~= nil
		)", prefab_name, animation_name));
		auto has_anim = tz_must(tz::lua_get_bool("has_anim"));
		if(has_anim)
		{
			tz_must(tz::lua_execute(std::format(R"(
			anim = c.{}
			_count = #anim.frames
			)", animation_name)));
			int frame_count = tz_must(tz::lua_get_int("_count"));
			int fps = tz_must(tz::lua_get_int("anim.fps"));
			bool loop = tz_must(tz::lua_get_bool("anim.loop"));

			flipbook = game::render::create_flipbook(fps, loop);
			for(std::size_t i = 0; i < frame_count; i++)
			{
				tz_must(tz::lua_execute(std::format("_tmp = _internal_index(anim.frames, {})", i + 1)));
				tz::error_code err = tz::lua_execute(std::format("_tmp_emissive = _internal_index(anim.emissive_frames, {})", i + 1));
				std::string emissive_path = "./res/images/transparent1x1.png";
				if(err == tz::error_code::success)
				{
					emissive_path = std::format("./res/images/{}", tz_must(tz::lua_get_string("_tmp_emissive")));
				}
				std::string path = std::format("./res/images/{}", tz_must(tz::lua_get_string("_tmp")));
				game::render::flipbook_add_frame(flipbook, game::render::create_image_from_file(path), game::render::create_image_from_file(emissive_path));
			}
		}
	}

	template<typename T>
	bool impl_collect_prefab_data(std::string_view prefab_name, const char* data_name, T& data)
	{
		tz_must(tz::lua_execute(std::format(R"(
			myval = prefabs.{}.{}
			myval_nil = myval == nil
			--if myval == nil then error("\"prefabs.{}.{}\" expected to be non-nil, but it's nil") end
		)", prefab_name, data_name, prefab_name, data_name)));
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

	int impl_get_prefab_data()
	{
		auto [prefab_name] = tz::lua_parse_args<std::string>();
		prefab& data = prefab_data[prefab_name];
		tz::lua_execute(std::format(R"(
			_internal_index = function(arr, idx) return arr[idx] end
			c = prefabs.{}
		)", prefab_name));
		data.name = prefab_name;
		impl_collect_prefab_animation(prefab_name, "idle", data.idle);
		impl_collect_prefab_animation(prefab_name, "move_horizontal", data.move_horizontal);
		impl_collect_prefab_animation(prefab_name, "move_up", data.move_up);
		impl_collect_prefab_animation(prefab_name, "move_down", data.move_down);
		impl_collect_prefab_animation(prefab_name, "cast", data.cast);
		impl_collect_prefab_animation(prefab_name, "attack", data.attack);
		impl_collect_prefab_data(prefab_name, "base_health", data.base_health);
		impl_collect_prefab_data(prefab_name, "movement_speed", data.movement_speed);
		impl_collect_prefab_data(prefab_name, "power", data.power);
		if(!impl_collect_prefab_data(prefab_name, "display_name", data.display_name))
		{
			data.display_name = data.name;
		}
		if(!impl_collect_prefab_data(prefab_name, "description", data.description))
		{
			data.description = "<No Description>";
		}
		impl_collect_prefab_data(prefab_name, "base_cooldown", data.base_cooldown);
		impl_collect_prefab_data(prefab_name, "base_damage", data.base_damage);
		impl_collect_prefab_data(prefab_name, "leeway_coefficient", data.leeway_coefficient);
		impl_collect_prefab_data(prefab_name, "face_move_direction", data.face_move_direction);
		impl_collect_prefab_data(prefab_name, "has_tooltip", data.has_tooltip);
		impl_collect_prefab_data(prefab_name, "combat", data.combat);
		impl_collect_prefab_data(prefab_name, "taunt", data.taunt);
		impl_collect_prefab_data(prefab_name, "ambush", data.ambush);
		impl_collect_prefab_data(prefab_name, "attackable", data.attackable);
		impl_collect_prefab_data(prefab_name, "spell_decoration", data.spell_decoration);
		impl_collect_prefab_data(prefab_name, "require_target_entity_to_play", data.require_target_entity_to_play);
		impl_collect_prefab_data(prefab_name, "require_target_location_to_play", data.require_target_location_to_play);
		float colour_tint_r = 1.0f;
		float colour_tint_g = 1.0f;
		float colour_tint_b = 1.0f;
		impl_collect_prefab_data(prefab_name, "colour_tint_r", colour_tint_r);
		impl_collect_prefab_data(prefab_name, "colour_tint_g", colour_tint_g);
		impl_collect_prefab_data(prefab_name, "colour_tint_b", colour_tint_b);
		float initial_scalex = 1.0f;
		float initial_scaley = 1.0f;
		impl_collect_prefab_data(prefab_name, "initial_scale_x", initial_scalex);
		impl_collect_prefab_data(prefab_name, "initial_scale_y", initial_scaley);
		data.initial_scale = {initial_scalex, initial_scaley};
		data.colour_tint = {colour_tint_r, colour_tint_g, colour_tint_b};
		impl_collect_prefab_data(prefab_name, "emissive", data.emissive);
		return 0;
	}
}
