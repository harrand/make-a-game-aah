#include "tz/core/vector.hpp"

namespace game
{
	////////////////////////////////////
	///////////// GAMEPLAY /////////////
	////////////////////////////////////
	// changing these will impact the main game loop

	// how much mana does a lvl 1 noob have?
	constexpr unsigned int config_default_max_mana = 100;
	// how much mana does a lvl 1 noob regen in a second?
	constexpr float config_default_mps = 5.0f;
	// how much would it cost to play a power 1 card?
	constexpr float config_mana_cost_per_power = 10.0f;

	// leeway is how far away an entity can be from another but still be considered "touching". leeway is proportional to the speed of the entity, infact equal to a % of it - this value between 0 and 1
	constexpr float config_global_leeway_dist = 0.5f;
	constexpr float config_default_aggro_range = 1.5f;
	
	constexpr unsigned int config_gold_per_power = 5;

	////////////////////////////////
	///////////// FEEL /////////////
	////////////////////////////////
	// tuning these will impact some small "juice" features, but nothing important

	// when the computer (ai) plays a card, how long should the process take, starting from the card moving from the deck, to the spell/creature actually appearing. too low = player doesnt have time to see the card played, too high = gameplay feels sluggish
	constexpr float config_computer_play_card_turnaround_time_seconds = 2.0f;
	// when the computer (ai) plays a card, how snappily should the card drag across the screen to the play position? higher means snappier
	constexpr float config_computer_play_card_drag_speed = 10.0f;

	/////////////////////////////////
	///////////// THEME /////////////
	/////////////////////////////////
	// changing these will change how the game composes and displays everything, which could affect gameplay in some circumstances

	// what colour aura should surround something aligned with the player
	constexpr tz::v3f config_player_aligned_colour = {0.0f, 0.0f, 0.5f};
	// what colour aura should surround something aligned with the enemy
	constexpr tz::v3f config_enemy_aligned_colour = {0.5f, 0.0f, 0.0f};
	// what colour is mana?
	constexpr tz::v3f config_mana_bar_colour = {0.1f, 0.2f, 0.8f};

	// where (relative to the screen) should the player avatar reside? should be between -1,1 (so {-1, -1} is bottom far left, {1, 1} is top far right)
	constexpr tz::v2f config_player_avatar_position = {-0.85f, 0.0f};
	// where (relative to the screen) should the enemy avatar reside? should be between -1,1 (so {-1, -1} is bottom far left, {1, 1} is top far right)
	constexpr tz::v2f config_enemy_avatar_position = {0.85f, 0.0f};

	// where (exact coordinates) should the enemy move cards when it plays them? possible TODO: enemy AI will choose a play position depending on what card its playing e.g a spell.
	constexpr tz::v2f config_enemy_play_position = {1.0f, 0.0f};

	// when an entity is set as either the player/enemy avatar, it ought to display slightly larger than normal - determined by this scale vector.
	constexpr tz::v2f config_avatar_scale = {1.0f, 1.0f};
	// how much (exact coords) offset should be applied to a tooltip relative to the entity its describing?
	constexpr tz::v2f config_entity_tooltip_offset = {0.0f, 0.2f};
	// how much (exact coords) offset should be applied to a tooltip relative to the card its describing?
	constexpr tz::v2f config_card_tooltip_offset = {0.0f, 0.3f};

	// how large is the text of the entity tooltip?
	constexpr float config_entity_tooltip_text_size = 0.025f;
	// how large is the text of the card tooltip?
	constexpr float config_card_tooltip_text_size = 0.03f;

	////////////////////////////////////
	///////////// INTERNAL /////////////
	////////////////////////////////////
	// changing these will almost certainly drastically affect gameplay, and doing it wrongly will break everything
	
	// internal multiplier (how much actual coordinate does an entity with speed 1.0 travel)
	constexpr float config_global_speed_multiplier = 0.35f;
	// internal multiplier (how much actual coordinate does an entity with scale 1.0 span)
	constexpr float config_global_uniform_scale = 0.15f;

	constexpr tz::v3f config_player_reticule_colour_on_entity = {1.0f, 0.0f, 0.0f};
	constexpr tz::v3f config_player_reticule_colour_on_location = {1.0f, 1.0f, 0.0f};
	constexpr float config_healthbar_duration = 5.0f;

}
