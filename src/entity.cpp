#include "entity.hpp"
#include "render.hpp"
#include "prefab.hpp"
#include <vector>

namespace game
{
	// soa
	// public
	std::vector<unsigned int> hps = {};
	std::vector<prefab> creatures = {};
	std::vector<tz::v2f> positions = {};
	std::vector<float> rotations = {};
	std::vector<tz::v2f> scales = {};
	std::size_t entity_count = 0;

	// private
	std::vector<render::handle> quads = {};

	// meta
	std::vector<entity_handle> entity_free_list = {};

	constexpr float global_uniform_scale = 0.2f;

	entity_handle create_entity(entity_info info)
	{
		info.scale *= global_uniform_scale;
		entity_handle ret;
		// recycle if we can
		if(entity_free_list.size())
		{
			ret = entity_free_list.back();
			entity_free_list.pop_back();
		}
		else // otherwise make a new one.
		{
			ret = static_cast<tz::hanval>(entity_count++);
			hps.push_back({});
			creatures.push_back({});
			positions.push_back({});
			rotations.push_back({});
			scales.push_back({});

			quads.push_back(tz::nullhand);
		}

		// set new data
		hps[ret.peek()] = info.hp;
		creatures[ret.peek()] = game::get_prefab(info.prefab_name);
		positions[ret.peek()] = info.position;
		rotations[ret.peek()] = info.rotation;
		scales[ret.peek()] = info.scale;

		quads[ret.peek()] = game::render::create_quad({.position = info.position, .rotation = info.rotation, .scale = info.scale});
		game::render::quad_set_flipbook(quads[ret.peek()], creatures[ret.peek()].idle);
		return ret;
	}

	void destroy_entity(entity_handle ent)
	{
		entity_free_list.push_back(ent);
		game::render::destroy_quad(quads[ent.peek()]);
	}

	void entity_update(float delta_seconds)
	{

	}

	void entity_start_casting(entity_handle ent)
	{
		game::render::quad_set_flipbook(quads[ent.peek()], creatures[ent.peek()].cast);
	}

	void entity_stop_casting(entity_handle ent)
	{
		game::render::quad_set_flipbook(quads[ent.peek()], creatures[ent.peek()].idle);
	}
}