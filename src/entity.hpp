#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP
#include "tz/core/vector.hpp"
#include "tz/core/handle.hpp"
#include <string>

namespace game
{
	struct entity_info
	{
		unsigned int hp = 1;
		std::string prefab_name;
		bool player_aligned = false;

		tz::v2f position;
		float rotation;
		tz::v2f scale = tz::v2f::filled(1.0f);
	};

	using entity_handle = tz::handle<entity_info>;
	entity_handle create_entity(entity_info info);
	void destroy_entity(entity_handle ent);
	void entity_update(float delta_seconds);

	tz::v2f entity_get_position(entity_handle ent);
	void entity_set_position(entity_handle ent, tz::v2f pos);

	void entity_face_left(entity_handle ent);
	void entity_face_right(entity_handle ent);
	void entity_start_casting(entity_handle ent);
	void entity_stop_casting(entity_handle ent);
	void entity_move(entity_handle ent, tz::v2f dir);

	void entity_set_target_location(entity_handle ent, tz::v2f location);
	void entity_set_target(entity_handle ent, entity_handle tar);
}

#endif // GAME_ENTITY_HPP