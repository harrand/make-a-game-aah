#ifndef GAME_UI_HPP
#define GAME_UI_HPP
#include "card.hpp"
#include "render.hpp"

namespace game
{
	namespace detail
	{
		struct ui_tag_t{};
	}

	using ui_handle = tz::handle<detail::ui_tag_t>;

	void ui_close_all();

	void ui_open_main_menu();
	void ui_close_main_menu();
	bool ui_main_menu_open();

	void ui_close_pause_menu();
	void ui_open_pause_menu();
	bool ui_pause_menu_opened();

	void ui_open_level_select();
	void ui_close_level_select();
	bool ui_level_select_open();

	void ui_open_deck_configure();
	void ui_close_deck_configure();
	bool ui_deck_configure_open();

	void ui_open_win_screen(card loot);
	void ui_close_win_screen();
	bool ui_win_screen_open();

	void ui_open_defeat_screen();
	void ui_close_defeat_screen();
	bool ui_defeat_screen_open();

	void ui_setup();
	void ui_advance();

	bool ui_covers_mouse();
}

#endif // GAME_UI_HPP

