#include "ui.hpp"
#include "level.hpp"
#include "tz/topaz.hpp"
#include "tz/os/input.hpp"
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
	constexpr short ui_base_layer = 64;
	constexpr tz::v3f quit_button_colour{1.0f, 0.7f, 0.6f};

	void ui_open_pause_menu()
	{
		tz_assert(!opened_pause_menu.has_value(), "pause menu already open!");
		opened_pause_menu = ui_element{};
		auto& elements = opened_pause_menu->contents;
		auto& text = opened_pause_menu->texts;

		elements["panel"] = game::render::create_quad({.scale = tz::v2f::filled(0.5f), .colour = {0.5f, 0.5f, 1.0f}, .layer = ui_base_layer});
		text["pause_text"] = game::render::create_text("kongtext", "Paused", {-0.2f, 0.35f}, {0.1f, 0.1f});

		elements["main_menu_button"] = game::render::create_quad({.position = {0.0f, 0.05f}, .scale = {0.15f, 0.065f}, .layer = ui_base_layer + 1});
		text["main_menu_text"] = game::render::create_text("kongtext", "Main Menu", {0.0f, 0.05f}, tz::v2f::filled(0.02f));

		elements["restart_button"] = game::render::create_quad({.position = {0.0f, -0.15f}, .scale = {0.15f, 0.065f}, .layer = ui_base_layer + 1});
		text["restart_text"] = game::render::create_text("kongtext", "Restart Level", {0.0f, -0.15f}, tz::v2f::filled(0.02f));

		// next is -0.15

		elements["unpause_button"] = game::render::create_quad({.position = {0.0f, -0.35f}, .scale = {0.15f, 0.065f}, .layer = ui_base_layer + 1});
		text["unpause_text"] = game::render::create_text("kongtext", "Resume", {0.0f, -0.35f}, tz::v2f::filled(0.02f));
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
					std::printf("pretend we just went to main menu :)");
					ui_close_pause_menu();
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
		}
	}
}
