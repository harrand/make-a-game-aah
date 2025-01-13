#include "save.hpp"
#include "tz/core/lua.hpp"
#include <filesystem>
#include <fstream>
#include <format>

namespace game
{
	std::filesystem::path save_path = "save/player.save";

	save_data s;

	void save()
	{
		s.prefab = game::get_player_prefab("player");
		if(!std::filesystem::exists(save_path.parent_path()))
		{
			std::filesystem::create_directory(save_path.parent_path());
		}
		std::ofstream file{save_path};

		std::string deck_str;
		for(std::string card : s.prefab.deck)
		{
			deck_str += std::format("\"{}\",\n", card);
		}
		file <<
			std::format(R"(
players.player =
{{
	avatar = "{}",
	deck =
	{{
{}
	}}
}}
			)", s.prefab.avatar_prefab, deck_str);
	}

	void load_save()
	{
		if(!std::filesystem::exists(save_path))
		{
			tz::lua_execute(R"(
players.player =
{
	avatar = "peasant",
	deck =
	{
		"peasant",
		"peasant",
		"peasant",
		"archer",
		"archer",
		"templar",
		"knight",
	}
}
			)");
		}
		tz::lua_execute_file(save_path);
		std::printf("successfully loaded save");
	}
}
