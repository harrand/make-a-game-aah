#include "entity.hpp"
#include "render.hpp"
#include "prefab.hpp"
#include <vector>

namespace game
{
	constexpr float global_speed_multiplier = 0.2f;
	// soa
	// public
	std::vector<float> speeds = {};
	std::vector<unsigned int> hps = {};
	std::vector<prefab> creatures = {};
	std::vector<tz::v2f> positions = {};
	std::vector<float> rotations = {};
	std::vector<tz::v2f> scales = {};
	std::size_t entity_count = 0;

	struct boolproxy{bool b;};
	// private
	std::vector<render::handle> quads = {};
	std::vector<tz::v2f> move_dirs = {};

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
			speeds.push_back({});
			hps.push_back({});
			creatures.push_back({});
			positions.push_back({});
			rotations.push_back({});
			scales.push_back({});

			quads.push_back(tz::nullhand);
			move_dirs.push_back(tz::v2f::zero());
		}

		// set new data
		hps[ret.peek()] = info.hp;
		creatures[ret.peek()] = game::get_prefab(info.prefab_name);
		positions[ret.peek()] = info.position;
		rotations[ret.peek()] = info.rotation;
		scales[ret.peek()] = info.scale;

		speeds[ret.peek()] = creatures[ret.peek()].movement_speed;

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
		for(std::size_t i = 0; i < entity_count; i++)
		{
			entity_handle ent = static_cast<tz::hanval>(i);
			if(std::find(entity_free_list.begin(), entity_free_list.end(), ent) != entity_free_list.end())
			{
				// in free list, do not update.
				continue;
			}

			// handle movement.
			tz::v2f move_dir = move_dirs[i];
			if(move_dir.length() > 0)
			{
				// moving in a direction. do the move and update anim.
				move_dir /= move_dir.length();
				move_dir *= speeds[i] * delta_seconds * global_speed_multiplier;
				game::render::quad_set_flipbook(quads[i], creatures[i].move_horizontal);
				auto scale = game::render::quad_get_scale(quads[i]);
				scale[0] = std::abs(scale[0]) * (move_dir[0] < 0.0f ? 1.0f : -1.0f);
				game::render::quad_set_scale(quads[i], scale);
				auto pos = game::render::quad_get_position(quads[i]);
				game::render::quad_set_position(quads[i], pos + move_dir);
			}
			else
			{
				// not moving. idle.
				game::render::quad_set_flipbook(quads[i], creatures[i].idle);
			}

			move_dirs[i] = tz::v2f::zero();
		}
	}

	void entity_start_casting(entity_handle ent)
	{
		game::render::quad_set_flipbook(quads[ent.peek()], creatures[ent.peek()].cast);
	}

	void entity_stop_casting(entity_handle ent)
	{
		game::render::quad_set_flipbook(quads[ent.peek()], creatures[ent.peek()].idle);
	}
	
	void entity_move(entity_handle ent, tz::v2f dir)
	{
		move_dirs[ent.peek()] += dir;
	}
}