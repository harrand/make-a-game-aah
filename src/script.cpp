#include "script.hpp"
#include "tz/core/job.hpp"
#include "tz/core/lua.hpp"
#include <filesystem>

namespace game
{
	void impl_local_script_init();
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
	}
}