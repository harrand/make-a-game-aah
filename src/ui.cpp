#include "ui.hpp"
#include "level.hpp"
#include "tz/topaz.hpp"
#include "tz/os/input.hpp"
#include "tz/os/window.hpp"
#include <unordered_map>
#include <vector>

namespace game
{
	struct ui_element
	{
		std::unordered_map<std::string, render::handle> contents = {};
		std::unordered_map<std::string, render::text_handle> texts = {};
		std::vector<ui_element> children = {};

		void close()
		{
			for(const auto& [name, quad] : this->contents)
			{
				game::render::destroy_quad(quad);
			}
			for(const auto& [name, text] : this->texts)
			{
				game::render::destroy_text(text);
			}
			for(auto& child : this->children)
			{
				child.close();
			}
		}
	};

	std::optional<ui_element> opened_pause_menu = std::nullopt;
	std::optional<ui_element> opened_level_select = std::nullopt;
	constexpr short ui_base_layer = 32;
	constexpr short ui_fg_layer = ui_base_layer * 2;
	constexpr tz::v3f quit_button_colour{1.0f, 0.7f, 0.6f};

	void ui_open_pause_menu()
	{
		tz_assert(!opened_pause_menu.has_value(), "pause menu already open!");
		opened_pause_menu = ui_element{};
		auto& elements = opened_pause_menu->contents;
		auto& text = opened_pause_menu->texts;

		elements["panel"] = game::render::create_quad({.scale = {0.5f, 0.7f}, .colour = {0.5f, 0.5f, 1.0f}, .layer = ui_base_layer});
		text["pause_text"] = game::render::create_text("kongtext", "Paused", {-0.2f, 0.35f}, {0.1f, 0.1f});

		elements["main_menu_button"] = game::render::create_quad({.position = {0.0f, 0.05f}, .scale = {0.15f, 0.065f}, .layer = ui_fg_layer});
		text["main_menu_text"] = game::render::create_text("kongtext", "Main Menu", {0.0f, 0.05f}, tz::v2f::filled(0.02f));

		elements["restart_button"] = game::render::create_quad({.position = {0.0f, -0.15f}, .scale = {0.15f, 0.065f}, .layer = ui_fg_layer});
		text["restart_text"] = game::render::create_text("kongtext", "Restart Level", {0.0f, -0.15f}, tz::v2f::filled(0.02f));

		elements["unpause_button"] = game::render::create_quad({.position = {0.0f, -0.35f}, .scale = {0.15f, 0.065f}, .layer = ui_fg_layer});
		text["unpause_text"] = game::render::create_text("kongtext", "Resume", {0.0f, -0.35f}, tz::v2f::filled(0.02f));

		elements["exit_button"] = game::render::create_quad({.position = {0.0f, -0.55f}, .scale = {0.15f, 0.065f}, .layer = ui_fg_layer});
		text["exit_text"] = game::render::create_text("kongtext", "Exit Game", {0.0f, -0.55f}, tz::v2f::filled(0.02f));
	}

	void ui_close_pause_menu()
	{
		tz_assert(opened_pause_menu.has_value(), "pause menu wasn't open");

		opened_pause_menu->close();
		opened_pause_menu = std::nullopt;
	}

	bool ui_pause_menu_opened()
	{
		return opened_pause_menu.has_value();
	}

	void ui_open_level_select()
	{
		tz_assert(!opened_level_select.has_value(), "level select already open!");
		opened_level_select = ui_element{};
		auto& elements = opened_level_select->contents;
		auto& text = opened_level_select->texts;

		elements["panel"] = game::render::create_quad({.scale = tz::v2f::filled(0.5f), .colour = {0.5f, 0.5f, 1.0f}, .layer = ui_base_layer});
		text["pause_text"] = game::render::create_text("kongtext", "Level Select", {-0.2f, 0.55f}, {0.1f, 0.1f});

		std::size_t count = 0;
		iterate_levels([&elements, &text, &count](std::string_view level_name, const level& data)
		{
			float y = 0.3f - (count++ * 0.14f);
			elements[std::format("{}_button", level_name)] = game::render::create_quad({.position = {0.0f, y}, .scale = {0.15f, 0.04f}, .layer = ui_fg_layer});
			text[std::format("{}_text", level_name)] = game::render::create_text("kongtext", std::string(level_name), {0.0f, y}, tz::v2f::filled(0.02f));
		});

		float closey = 0.3f - (count * 0.14f);
		elements["close_button"] = game::render::create_quad({.position = {0.0f, closey}, .scale = {0.15f, 0.065f}, .layer = ui_fg_layer});
		text["close_text"] = game::render::create_text("kongtext", "Close", {0.0f, closey}, tz::v2f::filled(0.02f));
	}

	void ui_close_level_select()
	{
		tz_assert(opened_level_select.has_value(), "level select wasn't open");

		opened_level_select->close();
		opened_level_select = std::nullopt;
	}

	bool ui_level_select_open()
	{
		return opened_level_select.has_value();
	}

	void ui_setup()
	{

	}

	void ui_advance()
	{
		if(opened_pause_menu.has_value())
		{
			game::render::handle unpause_button = opened_pause_menu->contents.at("unpause_button");
			bool unpause_moused_over = game::render::quad_is_mouseover(unpause_button);

			game::render::quad_set_colour(unpause_button, unpause_moused_over ? quit_button_colour * 1.1f : quit_button_colour);
			if(unpause_moused_over)
			{
				if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					ui_close_pause_menu();
					return;
				}
			}

			game::render::handle main_menu_button = opened_pause_menu->contents.at("main_menu_button");
			bool main_menu_moused_over = game::render::quad_is_mouseover(main_menu_button);
			game::render::quad_set_colour(main_menu_button, main_menu_moused_over ? quit_button_colour * 1.1f : quit_button_colour);

			if(main_menu_moused_over)
			{
				if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					ui_close_pause_menu();
					ui_open_level_select();
					return;
				}
			}

			game::render::handle restart_level_button = opened_pause_menu->contents.at("restart_button");
			bool restart_level_moused_over = game::render::quad_is_mouseover(restart_level_button);
			game::render::quad_set_colour(restart_level_button, restart_level_moused_over ? quit_button_colour * 1.1f : quit_button_colour);

			if(restart_level_moused_over)
			{
				if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					reload_level();
					ui_close_pause_menu();
					return;
				}
			}

			game::render::handle exit_button = opened_pause_menu->contents.at("exit_button");
			bool exit_moused_over = game::render::quad_is_mouseover(exit_button);
			game::render::quad_set_colour(exit_button, exit_moused_over ? quit_button_colour * 1.1f : quit_button_colour);

			if(exit_moused_over)
			{
				if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					tz::os::close_window();
					return;
				}
			}
		}

		if(opened_level_select.has_value())
		{
			game::render::handle close_button = opened_level_select->contents.at("close_button");
			bool close_moused_over = game::render::quad_is_mouseover(close_button);

			game::render::quad_set_colour(close_button, close_moused_over ? quit_button_colour * 1.1f : quit_button_colour);
			if(close_moused_over)
			{
				if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
				{
					ui_close_level_select();
					return;
				}
			}

			iterate_levels([](std::string_view level_name, const level& data)
			{
				if(!opened_level_select.has_value())
				{
					return;
				}
				game::render::handle level_button = opened_level_select->contents.at(std::format("{}_button", level_name));
				bool moused_over = game::render::quad_is_mouseover(level_button);
				game::render::quad_set_colour(level_button, moused_over ? quit_button_colour * 1.1f : quit_button_colour);
				if(moused_over)
				{
					if(tz::os::is_mouse_clicked(tz::os::mouse_button::left))
					{
						ui_close_level_select();
						load_level(data);
						return;
					}
				}
			});
		}
	}
}
