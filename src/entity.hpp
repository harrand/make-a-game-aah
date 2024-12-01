#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP
#include "tz/core/vector.hpp"
#include "tz/core/handle.hpp"
#include <string>
#include <span>
#include <functional>

namespace game
{
	namespace detail{struct entity_t{};}
	using entity_handle = tz::handle<detail::entity_t>;
	struct entity_info
	{
		unsigned int hp = -1u;
		std::string prefab_name;
		bool player_aligned = false;

		tz::v2f position;
		float rotation;
		tz::v2f scale = tz::v2f::filled(1.0f);
		entity_handle parent = tz::nullhand;
		void* userdata = nullptr;
	};

	entity_handle create_entity(entity_info info);
	void destroy_entity(entity_handle ent);
	void iterate_entities(std::function<void(entity_handle)> callback);
	void entity_update(float delta_seconds);

	tz::v2f entity_get_position(entity_handle ent);
	void entity_set_position(entity_handle ent, tz::v2f pos);

	short entity_get_layer(entity_handle ent);
	void entity_set_layer(entity_handle ent, short layer);

	tz::v2f entity_get_scale(entity_handle ent);
	void entity_set_scale(entity_handle ent, tz::v2f scale);

	entity_handle entity_get_parent(entity_handle ent);
	void entity_set_parent(entity_handle ent, entity_handle parent);

	tz::v3f entity_get_colour_tint(entity_handle ent);
	void entity_set_colour_tint(entity_handle ent, tz::v3f colour);

	unsigned int entity_get_hp(entity_handle ent);
	void entity_set_hp(entity_handle ent, unsigned int hp);
	unsigned int entity_get_max_hp(entity_handle ent);

	void entity_face_left(entity_handle ent);
	void entity_face_right(entity_handle ent);
	void entity_start_casting(entity_handle ent);
	void entity_stop_casting(entity_handle ent);
	void entity_move(entity_handle ent, tz::v2f dir);

	bool entity_is_player_aligned(entity_handle ent);
	void entity_set_is_player_aligned(entity_handle ent, bool player_aligned);

	std::span<const tz::v2f> entity_get_patrol(entity_handle ent);
	void entity_set_patrol(entity_handle ent, std::span<const tz::v2f> points);

	std::optional<tz::v2f> entity_get_target_location(entity_handle ent);
	void entity_set_target_location(entity_handle ent, tz::v2f location);
	entity_handle entity_get_target(entity_handle ent);
	void entity_set_target(entity_handle ent, entity_handle tar);

	void entity_display_tooltip(entity_handle ent);
	void entity_hide_tooltip(entity_handle ent);

	void* entity_get_userdata(entity_handle ent);
	void entity_set_userdata(entity_handle ent, void* userdata);
}

#endif // GAME_ENTITY_HPP