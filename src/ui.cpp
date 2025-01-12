#include "ui.hpp"
#include "level.hpp"
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
			this->contents["panel"] = game::render::create_quad({.scale = size, .colour = colour, .layer = ui_base_layer});
			this->texts["panel"] = game::render::create_text("kongtext", name, {-size[0] * 0.5f, size[1] * 0.8f}, tz::v2f::filled(0.1f));
		}

		void add_button(std::string name, tz::v2f position, bool small, std::function<bool(std::string_view)> on_click = nullptr)
		{
			auto handle = game::render::create_quad({.position = position, .scale = small ? tz::v2f{0.15f, 0.04f} : tz::v2f{0.15f, 0.065f}, .layer = ui_fg_layer});
			this->contents[std::format("button_{}", name)]  = handle;
			this->texts[std::format("button_{}", name)] = game::render::create_text("kongtext", name, position, tz::v2f::filled(0.02f));
			this->on_clicks[handle.peek()] = on_click;
		}

		void update()
		{
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
						if(this->on_clicks.at(handle.peek())(name.substr(button_prefix_len)))
						{
							break;
						}
					}
				}
			}
		}
	};

	std::optional<ui_element> opened_pause_menu = std::nullopt;
	std::optional<ui_element> opened_level_select = std::nullopt;

	void ui_open_pause_menu()
	{
		tz_assert(!opened_pause_menu.has_value(), "pause menu already open!");
		opened_pause_menu = ui_element{};

		opened_pause_menu->set_window("Paused", {0.5f, 0.7f});
		opened_pause_menu->add_button("Resume", {0.0f, 0.3f}, false, [](auto _)
				{
					ui_close_pause_menu();
					return true;
				});
		opened_pause_menu->add_button("Restart", {0.0f, 0.1f}, false, [](auto _)
				{
					ui_close_pause_menu();
					reload_level();
					return true;
				});
		opened_pause_menu->add_button("Main Menu", {0.0f, -0.1f}, false, [](auto _)
				{
					ui_close_pause_menu();
					ui_open_level_select();
					return true;
				});
		opened_pause_menu->add_button("Exit Game", {0.0f, -0.3f}, false, [](auto _)
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
			opened_level_select->add_button(name, {0.0f, y}, true, [](std::string_view name)
				{
					ui_close_level_select();
					game::load_level(game::get_level(std::string{name}));
					return true;
				});
		});

		float closey = 0.3f - (count * 0.14f);
		opened_level_select->add_button("Close", {0.0f, closey}, true, [](auto _)
			{
				ui_close_level_select();
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

	void ui_setup()
	{

	}

	void ui_advance()
	{
		if(opened_pause_menu.has_value())
		{
			opened_pause_menu->update();
		}
		if(opened_level_select.has_value())
		{
			opened_level_select->update();
		}
	}
}
