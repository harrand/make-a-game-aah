#include "script.hpp"

#include "entity.hpp"

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
		tz::lua_execute("prefabs = {}");
		for(const auto& entry : std::filesystem::directory_iterator("./script/prefabs"))
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
	}
}