#ifndef GAME_SAVE_HPP
#define GAME_SAVE_HPP
#include <string>
#include <vector>

namespace game
{
	struct save_data
	{
		std::string avatar;
		std::vector<std::string> cards = {};
	};

	void save(save_data s);

	void load_save();
}

#endif // GAME_SAVE_HPP
