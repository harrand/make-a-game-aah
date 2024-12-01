#include "script.hpp"
#include "entity.hpp"
#include "config.hpp"
#include "tz/core/job.hpp"
#include "tz/core/lua.hpp"
#include <filesystem>

namespace game
{
	void impl_local_script_init();
	void impl_create_game_api();
	void script_initialise()
	{
		std::vector<tz::job_handle> lua_init_jobs(tz::job_worker_count());
		for(std::size_t i = 0; i < tz::job_worker_count(); i++)
		{
			lua_init_jobs[i] = tz::job_execute_on([]()
			{
				impl_local_script_init();
			}, i);
		}
		for(tz::job_handle jh : lua_init_jobs)
		{
			tz::job_wait(jh);
		}
		impl_local_script_init();
	}

	void impl_local_script_init()
	{
		tz::lua_execute("prefabs = {}; prefabs.empty = {has_tooltip = false, combat = false}");
		for(const auto& entry : std::filesystem::recursive_directory_iterator("./script"))
		{
			if(entry.path().has_extension() && entry.path().extension() == ".lua")
			{
				tz_must(tz::lua_execute_file(entry.path()));
			}
		}
		impl_create_game_api();
	}

	void impl_create_game_api()
	{
		tz::lua_set_number("GLOBAL_UNIFORM_SCALE", config_global_uniform_scale);
		tz::lua_define_function("debuglog", []()
		{
			auto [msg] = tz::lua_parse_args<std::string>();
			std::string full_msg = std::format("[DEBUG] {}", msg);
			std::printf("%s", full_msg.c_str());
			return 0;
		});
		tz::lua_define_function("create_entity", []()
		{
			auto [prefab_name] = tz::lua_parse_args<std::string>();
			tz::v2f pos = tz::v2f::zero();
			if(tz::lua_stack_size() >= 3)
			{
				pos[0] = tz_must(tz::lua_stack_get_number(2));
				pos[1] = tz_must(tz::lua_stack_get_number(3));
			}

			entity_handle ret = game::create_entity({.prefab_name = prefab_name, .position = pos});

			tz::lua_push_int(ret.peek());
			return 1;
		});
		tz::lua_define_function("destroy_entity", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			game::destroy_entity(static_cast<tz::hanval>(ent));
			return 0;
		});

		tz::lua_define_function("entity_get_position", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			auto pos = game::entity_get_position(static_cast<tz::hanval>(ent));
			tz::lua_push_number(pos[0]);
			tz::lua_push_number(pos[1]);
			return 2;
		});

		tz::lua_define_function("entity_set_position", []()
		{
			auto [ent, x, y] = tz::lua_parse_args<std::int64_t, float, float>();

			game::entity_set_position(static_cast<tz::hanval>(ent), tz::v2f{x, y});
			return 0;
		});

		tz::lua_define_function("entity_get_layer", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			tz::lua_push_int(game::entity_get_layer(static_cast<tz::hanval>(ent)));
			return 1;
		});

		tz::lua_define_function("entity_set_layer", []()
		{
			auto [ent, layer] = tz::lua_parse_args<std::int64_t, std::int64_t>();
			game::entity_set_layer(static_cast<tz::hanval>(ent), layer);
			return 0;
		});

		tz::lua_define_function("entity_get_scale", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			auto sc = game::entity_get_scale(static_cast<tz::hanval>(ent));
			tz::lua_push_number(sc[0]);
			tz::lua_push_number(sc[1]);
			return 2;
		});

		tz::lua_define_function("entity_set_scale", []()
		{
			auto [ent, x, y] = tz::lua_parse_args<std::int64_t, float, float>();

			game::entity_set_scale(static_cast<tz::hanval>(ent), tz::v2f{x, y});
			return 0;
		});


		tz::lua_define_function("entity_get_parent", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();

			tz::lua_push_int(game::entity_get_parent(static_cast<tz::hanval>(ent)).peek());
			return 1;
		});

		tz::lua_define_function("entity_set_parent", []()
		{
			auto [ent, parent] = tz::lua_parse_args<std::int64_t, std::int64_t>();
			game::entity_set_parent(static_cast<tz::hanval>(ent), static_cast<tz::hanval>(parent));
			return 0;
		});

		tz::lua_define_function("entity_get_colour_tint", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();

			tz::v3f col = game::entity_get_colour_tint(static_cast<tz::hanval>(ent));	
			tz::lua_push_number(col[0]);
			tz::lua_push_number(col[1]);
			tz::lua_push_number(col[2]);
			return 3;
		});

		tz::lua_define_function("entity_set_colour_tint", []()
		{
			auto [ent, r, g, b] = tz::lua_parse_args<std::int64_t, float, float, float>();
			game::entity_set_colour_tint(static_cast<tz::hanval>(ent), {r, g, b});
			return 0;
		});

		tz::lua_define_function("entity_get_hp", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			tz::lua_push_int(game::entity_get_hp(static_cast<tz::hanval>(ent)));
			return 1;
		});

		tz::lua_define_function("entity_set_hp", []()
		{
			auto [ent, hp] = tz::lua_parse_args<std::int64_t, std::int64_t>();
			game::entity_set_hp(static_cast<tz::hanval>(ent), hp);
			return 0;
		});

		tz::lua_define_function("entity_get_max_hp", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			tz::lua_push_int(game::entity_get_max_hp(static_cast<tz::hanval>(ent)));
			return 1;
		});

		tz::lua_define_function("entity_face_left", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			game::entity_face_left(static_cast<tz::hanval>(ent));
			return 0;
		});

		tz::lua_define_function("entity_face_right", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			game::entity_face_right(static_cast<tz::hanval>(ent));
			return 0;
		});

		tz::lua_define_function("entity_move", []()
		{
			auto [ent, dirx, diry] = tz::lua_parse_args<std::int64_t, float, float>();
			game::entity_move(static_cast<tz::hanval>(ent), {dirx, diry});
			return 0;
		});

		tz::lua_define_function("entity_is_player_aligned", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			tz::lua_push_bool(game::entity_is_player_aligned(static_cast<tz::hanval>(ent)));
			return 1;
		});

		tz::lua_define_function("entity_set_is_player_aligned", []()
		{
			auto [ent, player_aligned] = tz::lua_parse_args<std::int64_t, bool>();
			game::entity_set_is_player_aligned(static_cast<tz::hanval>(ent), player_aligned);
			return 0;
		});

		tz::lua_define_function("entity_get_patrol", []()->int
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			auto patrol = game::entity_get_patrol(static_cast<tz::hanval>(ent));
			for(tz::v2f point : patrol)
			{
				tz::lua_push_number(point[0]);
				tz::lua_push_number(point[1]);
			}
			return patrol.size() * 2;
		});

		tz::lua_define_function("entity_set_patrol", []()
		{
			auto ent = tz_must(tz::lua_stack_get_int(1));
			std::vector<tz::v2f> patrol;
			for(std::size_t i = 2; i < tz::lua_stack_size(); i+=2)
			{
				patrol.push_back(tz::v2d{tz_must(tz::lua_stack_get_number(i)), tz_must(tz::lua_stack_get_number(i + 1))});
			}
			game::entity_set_patrol(static_cast<tz::hanval>(ent), patrol);
			return 0;
		});

		tz::lua_define_function("entity_get_target_location", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			auto maybe_loc = game::entity_get_target_location(static_cast<tz::hanval>(ent));
			if(maybe_loc.has_value())
			{
				tz::lua_push_number(maybe_loc.value()[0]);
				tz::lua_push_number(maybe_loc.value()[1]);
			}
			else
			{
				tz::lua_push_nil();
				tz::lua_push_nil();
			}
			return 2;
		});

		tz::lua_define_function("entity_set_target_location", []()
		{
			auto [ent, locx, locy] = tz::lua_parse_args<std::int64_t, float, float>();
			game::entity_set_target_location(static_cast<tz::hanval>(ent), {locx, locy});
			return 0;
		});

		tz::lua_define_function("entity_get_target", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			entity_handle tar = game::entity_get_target(static_cast<tz::hanval>(ent));
			if(tar == tz::nullhand)
			{
				tz::lua_push_nil();
			}
			else
			{
				tz::lua_push_int(tar.peek());
			}
			return 1;
		});

		tz::lua_define_function("entity_set_target", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			entity_handle tar = tz::nullhand;
			if(tz::lua_stack_size() >= 2)
			{
				tar = static_cast<tz::hanval>(tz_must(tz::lua_stack_get_int(2)));
			}
			game::entity_set_target(static_cast<tz::hanval>(ent), tar);
			return 0;
		});

		tz::lua_define_function("entity_get_userdata", []()
		{
			auto [ent] = tz::lua_parse_args<std::int64_t>();
			tz::lua_push_int(reinterpret_cast<std::uintptr_t>(game::entity_get_userdata(static_cast<tz::hanval>(ent))));
			return 1;
		});

		tz::lua_define_function("entity_set_userdata", []()
		{
			auto [ent, userdata] = tz::lua_parse_args<std::int64_t, std::int64_t>();
			game::entity_set_userdata(static_cast<tz::hanval>(ent), reinterpret_cast<void*>(static_cast<std::uintptr_t>(userdata)));
			return 0;
		});
	}
}