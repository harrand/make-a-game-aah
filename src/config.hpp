#include "tz/core/vector.hpp"

namespace game
{
	constexpr unsigned int config_default_max_mana = 100;
	constexpr float config_default_mps = 5.0f;
	constexpr float config_mana_cost_per_power = 10.0f;
	
	constexpr float config_computer_play_card_turnaround_time_seconds = 2.0f;
	constexpr float config_computer_play_card_drag_speed = 10.0f;

	constexpr tz::v3f config_player_aligned_colour = {0.0f, 0.0f, 0.5f};
	constexpr tz::v3f config_enemy_aligned_colour = {0.5f, 0.0f, 0.0f};

	constexpr tz::v3f config_mana_bar_colour = {0.1f, 0.2f, 0.8f};
}