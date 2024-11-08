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

		tz::v2f position;
		float rotation;
		tz::v2f scale = tz::v2f::filled(1.0f);
	};

	using entity_handle = tz::handle<entity_info>;
	entity_handle create_entity(entity_info info);
	void destroy_entity(entity_handle ent);
	void entity_update(float delta_seconds);

	void entity_start_casting(entity_handle ent);
	void entity_stop_casting(entity_handle ent);
}

#endif // GAME_ENTITY_HPP