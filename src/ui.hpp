#ifndef GAME_UI_HPP
#define GAME_UI_HPP
#include "render.hpp"

namespace game
{
	namespace detail
	{
		struct ui_tag_t{};
	}

	using ui_handle = tz::handle<detail::ui_tag_t>;

	void ui_close_pause_menu();
	void ui_open_pause_menu();
	bool ui_pause_menu_opened();

	void ui_open_level_select();
	void ui_close_level_select();
	bool ui_level_select_open();

	void ui_setup();
	void ui_advance();
}

#endif // GAME_UI_HPP

