#include "ui.hpp"
#include "level.hpp"
#include "prefab.hpp"
#include "player.hpp"
#include "save.hpp"
#include "config.hpp"
#include "tz/topaz.hpp"
#include "tz/os/input.hpp"
#include "tz/os/window.hpp"
#include <unordered_map>
#include <vector>

namespace game
{
	constexpr short ui_base_layer = 32;
	constexpr short ui_fg_layer = ui_base_layer * 2;
	constexpr tz::v3f quit_button_colour{1.0f, 0.7f, 0.6f};
	bool wait_for_next_mouse_release = false;
	bool wait_for_next_escape = false;

	enum class button_size
	{
		small,
		smallwide,
		medium,
		mediumwide,
		large,
		largewide
	};

	struct ui_element
	{
		std::unordered_map<std::string, render::handle> contents = {};
		std::unordered_map<std::string, render::text_handle> texts = {};
		std::unordered_map<std::uint64_t, std::function<bool(std::string_view)>> on_clicks = {};
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

		void set_window(std::string name, tz::v2f size, tz::v3f colour = {0.5f, 0.5f, 1.0f})
		{
			constexpr float default_big_text_size = 0.1f;
			float big_text_size = default_big_text_size;
			float ideal_text_size = size[0] / (name.size() * 0.7f);
			if(big_text_size > ideal_text_size)
			{
				big_text_size = ideal_text_size;
			}
			this->contents["panel"] = game::render::create_quad({.scale = size, .colour = colour, .layer = ui_base_layer});
			tz::v2f text_offset = {0.0f, 0.0f};
			text_offset[0] = -0.5f * big_text_size * name.size();
			text_offset[1] = size[1] * 0.8f;
			this->texts["panel"] = game::render::create_text("kongtext", name, text_offset, tz::v2f::filled(big_text_size));
		}

		static constexpr float text_size = 0.02f;
		void add_text(std::string text, tz::v2f position)
		{
			tz::v2f text_offset = {0.0f, 0.0f};
			text_offset[0] = -0.5f * text_size * text.size();
			this->texts[std::format("text_{}", text)] = game::render::create_text("kongtext", text, position + text_offset, tz::v2f::filled(text_size));
		}

		void add_button(std::string name, tz::v2f position, button_size size = button_size::medium, std::function<bool(std::string_view)> on_click = nullptr)
		{
			tz::v2f scale;
			tz::v2f panel_scale = game::render::quad_get_scale(this->contents.at("panel")) * 0.9f;
			switch(size)
			{
				case button_size::small:
					scale = {0.15f, 0.04f};
				break;
				case button_size::smallwide:
					scale = {panel_scale[0], 0.04f};
				break;
				case button_size::medium:
					scale = {0.2f, 0.065f};
				break;
				case button_size::mediumwide:
					scale = {panel_scale[0], 0.065f};
				break;
				case button_size::large:
					scale = {0.3f, 0.1f};
				break;
				case button_size::largewide:
					scale = {panel_scale[0], 0.1f};
				break;
			}
					//scale = game::render::quad_get_scale(this->contents.at("panel")) * 0.85f;
			auto handle = game::render::create_quad({.position = position, .scale = scale, .layer = ui_fg_layer});
			this->contents[std::format("button_{}", name)]  = handle;
			tz::v2f text_offset = {0.0f, 0.0f};
			text_offset[0] = -0.5f * text_size * name.size();
			this->texts[std::format("button_{}", name)] = game::render::create_text("kongtext", name, position + text_offset, tz::v2f::filled(text_size));
			this->on_clicks[handle.peek()] = on_click;
		}

		bool update()
		{
			bool retval = false;
			auto maybe_wnd = this->contents.find("panel");
			if(maybe_wnd != this->contents.end())
			{
				retval = game::render::quad_is_mouseover(maybe_wnd->second);
			}

			constexpr char button_prefix[] = "button_";
			auto button_prefix_len = std::strlen(button_prefix);
			for(const auto& [name, handle] : this->contents)
			{
				if(name.starts_with(button_prefix))
				{
					// its a button.
					bool moused_over = game::render::quad_is_mouseover(handle);
					game::render::quad_set_colour(handle, moused_over ? quit_button_colour * 1.1f : quit_button_colour);

					if(moused_over && tz::os::is_mouse_clicked(tz::os::mouse_button::left))
					{
						wait_for_next_mouse_release = true;
						if(this->on_clicks.at(handle.peek())(name.substr(button_prefix_len)))
						{
							break;
						}
					}
				}
			}
			return retval;
		}
	};

	std::optional<ui_element> opened_main_menu = std::nullopt;
	std::optional<ui_element> opened_pause_menu = std::nullopt;
	std::optional<ui_element> opened_level_select = std::nullopt;
	std::optional<ui_element> opened_deck_configure = std::nullopt;
	std::optional<ui_element> opened_win_screen = std::nullopt;
	std::optional<ui_element> opened_defeat_screen = std::nullopt;

	void ui_close_all()
	{
		if(opened_main_menu.has_value())
		{
			ui_close_main_menu();
		}
		if(opened_pause_menu.has_value())
		{
			ui_close_pause_menu();
		}
		if(opened_level_select.has_value())
		{
			ui_close_level_select();
		}
		if(opened_deck_configure.has_value())
		{
			ui_close_deck_configure();
		}
		if(opened_win_screen.has_value())
		{
			ui_close_win_screen();
		}
		if(opened_defeat_screen.has_value())
		{
			ui_close_defeat_screen();
		}
	}

	void ui_open_main_menu()
	{
		game::clear_level();
		tz_assert(!opened_main_menu.has_value(), "main menu already open!");
		opened_main_menu = ui_element{};

		opened_main_menu->set_window("Epic Card Game", {static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height(), 1.0f}, tz::v3f::filled(1.0f));
		game::render::quad_set_texture0(opened_main_menu->contents["panel"], game::render::create_image_from_file("./res/images/bgforest.png"));

		opened_main_menu->add_button("Continue", {0.0f, -0.7f}, button_size::medium, [](auto _)
				{
					ui_close_all();
					game::load_level(game::get_level("forest"));
					return true;
				});

		opened_main_menu->add_button("Quit", {0.0f, -0.9f}, button_size::medium, [](auto _)
				{
					ui_close_all();
					tz::os::close_window();
					return true;
				});

		opened_main_menu->add_button("Deck Configure", {0.0f, 0.3f}, button_size::medium, [](auto _)
				{
					ui_close_all();
					ui_open_deck_configure();
					return true;
				});

		opened_main_menu->add_button("Level Select", {0.0f, 0.5f}, button_size::medium, [](auto _)
				{
					ui_close_all();
					ui_open_level_select();
					return true;
				});
	}

	void ui_close_main_menu()
	{
		tz_assert(opened_main_menu.has_value(), "main menu wasn't open");

		opened_main_menu->close();
		opened_main_menu = std::nullopt;
	}

	bool ui_main_menu_open()
	{
		return opened_main_menu.has_value();
	}

	void ui_open_pause_menu()
	{
		tz_assert(!opened_pause_menu.has_value(), "pause menu already open!");
		opened_pause_menu = ui_element{};

		opened_pause_menu->set_window("Paused", {0.5f, 0.7f});
		opened_pause_menu->add_button("Resume", {0.0f, 0.3f}, button_size::medium, [](auto _)
				{
					ui_close_pause_menu();
					return true;
				});
		opened_pause_menu->add_button("Restart", {0.0f, 0.1f}, button_size::medium, [](auto _)
				{
					ui_close_pause_menu();
					reload_level();
					return true;
				});
		opened_pause_menu->add_button("Main Menu", {0.0f, -0.1f}, button_size::medium, [](auto _)
				{
					game::clear_level();
					ui_close_all();
					ui_open_main_menu();
					return true;
				});
		opened_pause_menu->add_button("Exit Game", {0.0f, -0.3f}, button_size::medium, [](auto _)
				{
					ui_close_pause_menu();
					tz::os::close_window();
					return true;
				});
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
		opened_level_select->set_window("Level Select", {0.5f, 0.5f});

		std::size_t count = 0;
		iterate_levels([&count](std::string_view level_name, const level& data)
		{
			float y = 0.3f - (count++ * 0.14f);
			std::string name{level_name};
			opened_level_select->add_button(name, {0.0f, y}, button_size::smallwide, [](std::string_view name)
				{
					ui_close_level_select();
					game::load_level(game::get_level(std::string{name}));
					return true;
				});
		});

		float closey = 0.3f - (count * 0.14f);
		opened_level_select->add_button("Back", {0.0f, closey}, button_size::medium, [](auto _)
			{
				ui_close_all();
				ui_open_main_menu();
				return true;
			});
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

	void ui_open_deck_configure()
	{
		tz_assert(!opened_deck_configure.has_value(), "deck configure already open!");
		opened_deck_configure = ui_element{};
		opened_deck_configure->set_window("Deck Configuration", {static_cast<float>(tz::os::window_get_width()) / tz::os::window_get_height(), 1.0f});

		opened_deck_configure->add_button("Confirm", {0.0f, -0.6f}, button_size::medium, [](auto _)
			{
				ui_close_all();
				ui_open_main_menu();
				return true;
			});
	}

	void ui_close_deck_configure()
	{
		tz_assert(opened_deck_configure.has_value(), "deck configure wasn't open");

		opened_deck_configure->close();
		opened_deck_configure = std::nullopt;
	}

	bool ui_deck_configure_open()
	{
		return opened_deck_configure.has_value();
	}

	void ui_open_win_screen(card loot)
	{
		tz_assert(!opened_win_screen.has_value(), "win screen already open!");
		opened_win_screen = ui_element{};
		opened_win_screen->set_window("Victory!", {0.9f, 0.9f});
		if(loot != card{})
		{
			opened_win_screen->add_text(std::format("You win: \"{}\"", game::get_prefab(loot.name).display_name), {0.0f, 0.5f});
			auto cardsprite = game::create_card_sprite(loot, false);
			opened_win_screen->contents["card loot"] = cardsprite;
			game::render::quad_set_layer(cardsprite, ui_fg_layer);
		}

		opened_win_screen->add_button("Collect", {0.0f, -0.6f}, button_size::medium, [loot](auto _)
			{
				game::get_player_prefab("player").deck.push_back(loot.name);
				game::save();
				ui_close_all();
				ui_open_main_menu();
				return true;
			});

		unsigned int sell_price = game::get_prefab(loot.name).power * config_gold_per_power;
		std::string sell_name = std::format("Sell ({} coins)", sell_price);
		opened_win_screen->add_button(sell_name, {0.0f, -0.4f}, button_size::medium, [loot, sell_price](auto _)
			{
				game::real_player_set_gold(game::real_player_get_gold() + sell_price);
				game::save();
				ui_close_all();
				ui_open_main_menu();
				return true;
			});
	}

	void ui_close_win_screen()
	{
		tz_assert(opened_win_screen.has_value(), "win screen wasn't open");

		opened_win_screen->close();
		opened_win_screen = std::nullopt;
	}

	bool ui_win_screen_open()
	{
		return opened_win_screen.has_value();
	}

	void ui_open_defeat_screen()
	{
		tz_assert(!opened_defeat_screen.has_value(), "defeat screen already open!");
		opened_defeat_screen = ui_element{};
		opened_defeat_screen->set_window("Defeat!", {0.9f, 0.9f});

		opened_defeat_screen->add_button("Main Menu", {0.0f, -0.6f}, button_size::medium, [](auto _)
			{
				ui_close_all();
				ui_open_main_menu();
				return true;
			});
	}

	void ui_close_defeat_screen()
	{
		tz_assert(opened_defeat_screen.has_value(), "defeat screen wasn't open");

		opened_defeat_screen->close();
		opened_defeat_screen = std::nullopt;
	}

	bool ui_defeat_screen_open()
	{
		return opened_defeat_screen.has_value();
	}

	void ui_setup()
	{

	}

	bool ui_mouse = false;

	void ui_advance()
	{
		ui_mouse = false;
		const bool escape_down = tz::os::is_key_pressed(tz::os::key::escape);
		if(wait_for_next_escape)
		{
			if(!escape_down)
			{
				wait_for_next_escape = false;
			}
		}
		else
		{
			if(escape_down && !ui_main_menu_open())
			{
				if(ui_pause_menu_opened())
				{
					ui_close_pause_menu();
				}
				else
				{
					ui_open_pause_menu();
				}
				wait_for_next_escape = true;
			}
		}
		if(wait_for_next_mouse_release)
		{
			if(!tz::os::is_mouse_clicked(tz::os::mouse_button::left))
			{
				wait_for_next_mouse_release = false;
			}
		}
		else
		{
			if(opened_main_menu.has_value())
			{
				auto v = opened_main_menu->update();
				ui_mouse = ui_mouse || v;
			}
			if(opened_pause_menu.has_value())
			{
				auto v = opened_pause_menu->update();
				ui_mouse = ui_mouse || v;
			}
			if(opened_level_select.has_value())
			{
				auto v = opened_level_select->update();
				ui_mouse = ui_mouse || v;
			}
			if(opened_deck_configure.has_value())
			{
				auto v = opened_deck_configure->update();
				ui_mouse = ui_mouse || v;
			}
			if(opened_win_screen.has_value())
			{
				auto v = opened_win_screen->update();
				ui_mouse = ui_mouse || v;
			}
			if(opened_defeat_screen.has_value())
			{
				auto v = opened_defeat_screen->update();
				ui_mouse = ui_mouse || v;
			}
		}
	}

	bool ui_covers_mouse()
	{
		return ui_mouse;
	}
}
