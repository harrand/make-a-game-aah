#include "entity.hpp"
#include "render.hpp"
#include "prefab.hpp"
#include "tz/core/lua.hpp"
#include <vector>

namespace game
{
	constexpr float global_speed_multiplier = 0.35f;
	constexpr float leeway_dist = 0.3f;
	// soa
	// public
	std::vector<float> speeds = {};
	std::vector<unsigned int> hps = {};
	std::vector<prefab> creatures = {};
	std::vector<bool> player_aligneds = {};
	std::vector<tz::v2f> positions = {};
	std::vector<float> rotations = {};
	std::vector<tz::v2f> scales = {};
	std::vector<entity_handle> parents = {};
	std::vector<void*> userdatas = {};
	std::vector<std::vector<tz::v2f>> patrols = {};
	std::vector<std::size_t> patrol_cursors = {};
	std::size_t entity_count = 0;

	struct boolproxy{bool b;};
	// private
	std::vector<render::handle> quads = {};
	std::vector<tz::v2f> move_dirs = {};
	std::vector<std::optional<tz::v2f>> target_locations = {};
	std::vector<entity_handle> targets = {};
	std::vector<std::vector<entity_handle>> childrens = {};
	std::vector<render::text_handle> tooltips = {};

	// meta
	std::vector<entity_handle> entity_free_list = {};

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
			player_aligneds.push_back(false);
			positions.push_back({});
			rotations.push_back({});
			scales.push_back({});
			parents.push_back({});
			userdatas.push_back(nullptr);
			patrols.push_back({});
			patrol_cursors.push_back(0);

			quads.push_back(tz::nullhand);
			move_dirs.push_back(tz::v2f::zero());
			target_locations.push_back(std::nullopt);
			targets.push_back(tz::nullhand);
			childrens.push_back({});
			tooltips.push_back(tz::nullhand);
		}

		// set new data
		auto& prefab = creatures[ret.peek()];
		prefab = game::get_prefab(info.prefab_name);

		hps[ret.peek()] = info.hp == -1u ? prefab.base_health : info.hp;

		player_aligneds[ret.peek()] = info.player_aligned;
		positions[ret.peek()] = info.position;
		rotations[ret.peek()] = info.rotation;
		scales[ret.peek()] = info.scale;
		parents[ret.peek()] = info.parent;
		userdatas[ret.peek()] = info.userdata;
		patrols[ret.peek()] = {};
		patrol_cursors[ret.peek()] = 0;
		if(info.parent != tz::nullhand)
		{
			childrens[info.parent.peek()].push_back(ret);
		}
		childrens[ret.peek()] = {};

		speeds[ret.peek()] = prefab.movement_speed;

		quads[ret.peek()] = game::render::create_quad({.position = info.position, .rotation = info.rotation, .scale = info.scale});
		game::render::quad_set_flipbook(quads[ret.peek()], prefab.idle);

		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_create; if fn ~= nil then fn({}) end", prefab.name, ret.peek())));
		return ret;
	}

	void destroy_entity(entity_handle ent)
	{
		auto prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_destroy; if fn ~= nil then fn({}) end", prefab.name, ent.peek())));

		entity_set_parent(ent, tz::nullhand);

		entity_free_list.push_back(ent);
		game::render::destroy_quad(quads[ent.peek()]);
		if(tooltips[ent.peek()] != tz::nullhand)
		{
			game::render::destroy_text(tooltips[ent.peek()]);
			tooltips[ent.peek()] = tz::nullhand;
		}
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

			if(game::render::quad_is_mouseover(quads[ent.peek()]))
			{
				entity_display_tooltip(ent);
			}
			else
			{
				entity_hide_tooltip(ent);
			}

			auto prefab = creatures[ent.peek()];
			tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_update; if fn ~= nil then fn({}, 0.0 + {}) end", prefab.name, ent.peek(), delta_seconds)));

			// handle target ent/loc
			auto pos = game::render::quad_get_position(quads[i]);
			auto maybe_tarloc = target_locations[i];
			entity_handle tar = targets[i];
			const float leeway = global_speed_multiplier * speeds[i] * leeway_dist;
			if(!maybe_tarloc.has_value() && tar != tz::nullhand)
			{
				tz::v2f tar_pos = game::render::quad_get_position(quads[tar.peek()]);
				if((tar_pos - pos).length() > leeway)
				{
					maybe_tarloc = tar_pos;
				}
				else
				{
					targets[i] = tz::nullhand;
				}
			}
			if(maybe_tarloc.has_value())
			{
				if((maybe_tarloc.value() - pos).length() > leeway)
				{
					entity_move(ent, maybe_tarloc.value() - pos);	
				}
				else
				{
					if(patrols[i].size())
					{
						patrol_cursors[i] = (patrol_cursors[i] + 1) % patrols[i].size();
						target_locations[i] = patrols[i][patrol_cursors[i]];
					}
					else
					{
						target_locations[i] = std::nullopt;
					}
				}
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
				if(move_dir[0] < 0.0f)
				{
					scale[0] = -std::abs(scale[0]);
				}
				else if(move_dir[0] > 0.0f)
				{
					scale[0] = std::abs(scale[0]);
				}
				game::render::quad_set_scale(quads[i], scale);
				auto pos = game::render::quad_get_position(quads[i]);
				entity_set_position(static_cast<tz::hanval>(i), pos + move_dir);
			}
			else
			{
				// not moving. idle.
				//game::render::quad_set_flipbook(quads[i], creatures[i].idle);
			}

			move_dirs[i] = tz::v2f::zero();
		}
	}

	tz::v2f entity_get_position(entity_handle ent)
	{
		auto pos = positions[ent.peek()];
		if(parents[ent.peek()] != tz::nullhand)
		{
			pos += entity_get_position(parents[ent.peek()]);
		}
		return pos;
	}

	void entity_set_position(entity_handle ent, tz::v2f pos)
	{
		positions[ent.peek()] = pos;
		game::render::quad_set_position(quads[ent.peek()], entity_get_position(ent));

		for(entity_handle child : childrens[ent.peek()])
		{
			entity_set_position(child, positions[child.peek()]);
		}
	}

	short entity_get_layer(entity_handle ent)
	{
		return game::render::quad_get_layer(quads[ent.peek()]);
	}

	void entity_set_layer(entity_handle ent, short layer)
	{
		game::render::quad_set_layer(quads[ent.peek()], layer);
	}

	tz::v2f entity_get_scale(entity_handle ent)
	{
		return scales[ent.peek()];
	}

	void entity_set_scale(entity_handle ent, tz::v2f scale)
	{
		scale *= global_uniform_scale;
		scales[ent.peek()] = scale;
		game::render::quad_set_scale(quads[ent.peek()], scale);
	}

	entity_handle entity_get_parent(entity_handle ent)
	{
		return parents[ent.peek()];
	}

	void entity_set_parent(entity_handle ent, entity_handle parent)
	{
		auto old_parent = parents[ent.peek()];
		if(old_parent != tz::nullhand)
		{
			std::erase(childrens[old_parent.peek()], ent);
		}
		parents[ent.peek()] = parent;
		// local position is unchanged.
		childrens[parent.peek()].push_back(ent);
		// however need to update our position now we have a parent, we do that by setting our local position to itself.
		entity_set_position(ent, positions[ent.peek()]);
	}

	tz::v3f entity_get_colour_tint(entity_handle ent)
	{
		return game::render::quad_get_colour(quads[ent.peek()]);
	}

	void entity_set_colour_tint(entity_handle ent, tz::v3f colour)
	{
		game::render::quad_set_colour(quads[ent.peek()], colour);
	}

	unsigned int entity_get_hp(entity_handle ent)
	{
		return hps[ent.peek()];
	}

	void entity_set_hp(entity_handle ent, unsigned int hp)
	{
		hps[ent.peek()] = std::clamp(hp, 0u, entity_get_max_hp(ent));
	}

	unsigned int entity_get_max_hp(entity_handle ent)
	{
		return creatures[ent.peek()].base_health;
	}

	void entity_face_left(entity_handle ent)
	{
		auto sc = game::render::quad_get_scale(quads[ent.peek()]);
		sc[0] = -std::abs(sc[0]);
		game::render::quad_set_scale(quads[ent.peek()], sc);
	}

	void entity_face_right(entity_handle ent)
	{
		auto sc = game::render::quad_get_scale(quads[ent.peek()]);
		sc[0] = std::abs(sc[0]);
		game::render::quad_set_scale(quads[ent.peek()], sc);
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

	std::span<const tz::v2f> entity_get_patrol(entity_handle ent)
	{
		return patrols[ent.peek()];
	}

	void entity_set_patrol(entity_handle ent, std::span<const tz::v2f> points)
	{
		auto& list = patrols[ent.peek()];
		list.clear();
		list.resize(points.size());	
		std::copy(points.begin(), points.end(), list.begin());
		patrol_cursors[ent.peek()] = 0;
		if(!entity_get_target_location(ent).has_value())
		{
			entity_set_target_location(ent, points.front());
		}
	}

	std::optional<tz::v2f> entity_get_target_location(entity_handle ent)
	{
		return target_locations[ent.peek()];
	}

	void entity_set_target_location(entity_handle ent, tz::v2f location)
	{
		target_locations[ent.peek()] = location;
		targets[ent.peek()] = tz::nullhand;
	}

	entity_handle entity_get_target(entity_handle ent)
	{
		return targets[ent.peek()];
	}

	void entity_set_target(entity_handle ent, entity_handle tar)
	{
		target_locations[ent.peek()] = std::nullopt;
		targets[ent.peek()] = tar;
	}

	void entity_display_tooltip(entity_handle ent)
	{
		if(!creatures[ent.peek()].has_tooltip)
		{
			return;
		}
		auto quad = quads[ent.peek()];
		tz::v2f tooltip_position = game::render::quad_get_position(quad);
		float yoffset = -0.2f;
		tooltip_position[0] -= 0.1f;
		if(tooltip_position[1] > (1.0f + yoffset))
		{
			tooltip_position[1] -= yoffset;
		}
		else
		{
			tooltip_position[1] += yoffset;
		}

		if(tooltips[ent.peek()] == tz::nullhand)
		{
			const bool is_player_aligned = player_aligneds[ent.peek()];
			std::string txt = std::format("{}", creatures[ent.peek()].display_name);
			tooltips[ent.peek()] = game::render::create_text("kongtext", txt, tooltip_position, tz::v2f::filled(0.025f), is_player_aligned ? tz::v3f{0.0f, 0.0f, 0.5f} : tz::v3f{0.5f, 0.0f, 0.0f});
		}
		else
		{
			game::render::text_set_position(tooltips[ent.peek()], tooltip_position);
		}
	}

	void entity_hide_tooltip(entity_handle ent)
	{
		if(tooltips[ent.peek()] != tz::nullhand)
		{
			game::render::destroy_text(tooltips[ent.peek()]);
			tooltips[ent.peek()] = tz::nullhand;
		}
	}

	void* entity_get_userdata(entity_handle ent)
	{
		return userdatas[ent.peek()];
	}

	void entity_set_userdata(entity_handle ent, void* userdata)
	{
		userdatas[ent.peek()] = userdata;
	}
}