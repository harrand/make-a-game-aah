#include "entity.hpp"
#include "render.hpp"
#include "prefab.hpp"
#include "config.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "tz/core/lua.hpp"
#include <vector>

namespace game
{
	// soa
	// public
	std::vector<float> speeds = {};
	std::vector<unsigned int> hps = {};
	std::vector<prefab> creatures = {};
	std::vector<bool> player_aligneds = {};
	std::vector<tz::v2f> positions = {};
	std::vector<float> rotations = {};
	std::vector<tz::v2f> scales = {};
	std::vector<float> cooldowns = {};
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
	std::vector<entity_handle> healthbars = {};
	std::vector<float> healthbar_timeouts = {};
	std::vector<bool> busys = {};

	// meta
	std::vector<entity_handle> entity_free_list = {};

	void impl_melee_attack(entity_handle lhs, entity_handle rhs);
	void impl_on_death(entity_handle ent);

	entity_handle create_entity(entity_info info)
	{
		info.scale *= config_global_uniform_scale;
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
			cooldowns.push_back(0.0f);
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
			healthbars.push_back(tz::nullhand);
			healthbar_timeouts.push_back(0.0f);
			busys.push_back(false);
		}

		// set new data
		auto& prefab = creatures[ret.peek()];
		prefab = game::get_prefab(info.prefab_name);

		hps[ret.peek()] = info.hp == -1u ? prefab.base_health : info.hp;

		player_aligneds[ret.peek()] = info.player_aligned;
		positions[ret.peek()] = info.position;
		rotations[ret.peek()] = info.rotation;
		scales[ret.peek()] = info.scale;
		cooldowns[ret.peek()] = 0.0f;
		userdatas[ret.peek()] = info.userdata;
		patrols[ret.peek()] = {};
		patrol_cursors[ret.peek()] = 0;
		parents[ret.peek()] = tz::nullhand;
		childrens[ret.peek()] = {};
		busys[ret.peek()] = false;
		targets[ret.peek()] = tz::nullhand;
		target_locations[ret.peek()] = std::nullopt;

		speeds[ret.peek()] = prefab.movement_speed;

		quads[ret.peek()] = game::render::create_quad({.position = info.position, .rotation = info.rotation, .scale = info.scale});
		game::render::quad_set_flipbook(quads[ret.peek()], prefab.idle);
		move_dirs[ret.peek()] = tz::v2f::zero();

		healthbar_timeouts[ret.peek()] = 0.0f;

		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_create; if fn ~= nil then fn({}) end", prefab.name, ret.peek())));
		if(info.parent != tz::nullhand)
		{
			entity_set_parent(ret, info.parent);
		}
		return ret;
	}

	void destroy_entity(entity_handle ent)
	{
		auto prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_destroy; if fn ~= nil then fn({}) end", prefab.name, ent.peek())));

		entity_set_parent(ent, tz::nullhand);

		auto children = childrens[ent.peek()];
		for(auto child : children)
		{
			entity_set_parent(child, tz::nullhand);
		}

		entity_free_list.push_back(ent);
		game::render::destroy_quad(quads[ent.peek()]);
		if(tooltips[ent.peek()] != tz::nullhand)
		{
			game::render::destroy_text(tooltips[ent.peek()]);
			tooltips[ent.peek()] = tz::nullhand;
		}
		if(healthbars[ent.peek()] != tz::nullhand)
		{
			destroy_entity(healthbars[ent.peek()]);
			healthbars[ent.peek()] = tz::nullhand;
		}
	}

	void iterate_entities(std::function<void(entity_handle)> callback)
	{
		for(std::size_t i = 0; i < entity_count; i++)
		{
			entity_handle ent = static_cast<tz::hanval>(i);
			if(std::find(entity_free_list.begin(), entity_free_list.end(), ent) != entity_free_list.end())
			{
				// in free list, do not run.
				continue;
			}
			callback(ent);
		}
	}

	void entity_update(float delta_seconds)
	{
		iterate_entities([delta_seconds](entity_handle ent)
		{
			auto i = ent.peek();
			if(cooldowns[ent.peek()] > 0.0f)
			{
				cooldowns[ent.peek()] -= delta_seconds;
			}

			if(game::render::quad_is_mouseover(quads[ent.peek()]))
			{
				entity_display_tooltip(ent);
			}
			else
			{
				entity_hide_tooltip(ent);
			}

			// healthbar timeouts
			if(healthbars[ent.peek()] != tz::nullhand)
			{
				float& timeout = healthbar_timeouts[ent.peek()];
				if((timeout -= delta_seconds) <= 0.0f)
				{
					destroy_entity(healthbars[ent.peek()]);
					healthbars[ent.peek()] = tz::nullhand;
				}
			}

			auto prefab = creatures[ent.peek()];
			tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_update; if fn ~= nil then fn({}, 0.0 + {}) end", prefab.name, ent.peek(), delta_seconds)));

			// handle target ent/loc
			auto pos = game::render::quad_get_position(quads[i]);
			auto maybe_tarloc = target_locations[i];
			entity_handle tar = targets[i];
			const float leeway = config_global_speed_multiplier * speeds[i] * config_global_leeway_dist;
			// if we dont have a target location but we do have a target entity, set tarloc (or attack if we're in range)
			if(!maybe_tarloc.has_value() && tar != tz::nullhand)
			{
				tz::v2f tar_pos = game::render::quad_get_position(quads[tar.peek()]);
				if((tar_pos - pos).length() > leeway)
				{
					maybe_tarloc = tar_pos;
				}
				else
				{
					//targets[i] = tz::nullhand;
					if(tar != tz::nullhand)
					{
						impl_melee_attack(ent, tar);
					}
				}
			}
			// do we have a target location?
			if(maybe_tarloc.has_value())
			{
				if((maybe_tarloc.value() - pos).length() > leeway)
				{
					entity_move(ent, maybe_tarloc.value() - pos);	
				}
				else
				{
					// reached location
					if(patrols[i].size())
					{
						// next patrol waypoint
						patrol_cursors[i] = (patrol_cursors[i] + 1) % patrols[i].size();
						target_locations[i] = patrols[i][patrol_cursors[i]];
					}
					else
					{
						// stop chasing.
						target_locations[i] = std::nullopt;
					}
				}
			}
			else if(tar == tz::nullhand)
			{
				// no target location. do we have a patrol?
				if(patrols[i].size())
				{
					target_locations[i] = patrols[i][patrol_cursors[i]];
				}
			}

			// handle movement.
			tz::v2f move_dir = move_dirs[i];
			if(hps[i] > 0.0f)
			{
				if(move_dir.length() > 0)
				{
					// moving in a direction. do the move and update anim.
					move_dir /= move_dir.length();
					move_dir *= speeds[i] * delta_seconds * config_global_speed_multiplier;
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
					auto pos = entity_get_position(ent);
					entity_set_position(static_cast<tz::hanval>(i), pos + move_dir);
				}
				else if(!busys[i])
				{
					if(player_aligneds[i] && creatures[i].combat)
					{
						// do something
						if(player_get_target() != tz::nullhand)
						{
							entity_set_target(ent, player_get_target());
						}
						else if(player_get_target_location().has_value())
						{
							entity_set_target_location(ent, player_get_target_location().value());
						}
					}
					// not moving and not doing anything idle.
					game::render::quad_set_flipbook(quads[i], creatures[i].idle);
				}
			}

			move_dirs[i] = tz::v2f::zero();
		});
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
		scales[ent.peek()] = scale;
		scale *= config_global_uniform_scale;
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
		if(parent != tz::nullhand)
		{
			childrens[parent.peek()].push_back(ent);
		}
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
		busys[ent.peek()] = true;
		game::render::quad_set_flipbook(quads[ent.peek()], creatures[ent.peek()].cast);
	}

	void entity_stop_casting(entity_handle ent)
	{
		busys[ent.peek()] = false;
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
		std::string txt = std::format("{}", creatures[ent.peek()].display_name);
		tz::v2u text_dims = tz::v2u::zero();
		unsigned int counterx = 0;
		for(char c : txt)
		{
			if(c != '\n')
			{
				text_dims[0] = std::max(text_dims[0], ++counterx);
			}
			else
			{
				// new line
				text_dims[1]++;
				counterx = 0;
			}
		}
		auto quad = quads[ent.peek()];
		tz::v2f tooltip_position = game::render::quad_get_position(quad);
		for(std::size_t i = 0; i < 2; i++)
		{
			float off = config_entity_tooltip_offset[i];
			if(i == 0)
			{
				off = -off;
			}
			if(off == 0.0f){continue;}
			if(tooltip_position[i] > (1.0f - off))
			{
				tooltip_position[i] += off;
			}
			else
			{
				tooltip_position[i] -= off;
			}
		}
		tooltip_position += (static_cast<tz::v2f>(text_dims) * -config_entity_tooltip_text_size * 0.5f);

		if(tooltips[ent.peek()] == tz::nullhand)
		{
			const bool is_player_aligned = player_aligneds[ent.peek()];
			tooltips[ent.peek()] = game::render::create_text("kongtext", txt, tooltip_position, tz::v2f::filled(config_entity_tooltip_text_size), is_player_aligned ? config_player_aligned_colour : config_enemy_aligned_colour);
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

	void impl_melee_attack(entity_handle lhs, entity_handle rhs)
	{
		if(cooldowns[lhs.peek()] <= 0.0f && hps[lhs.peek()] > 0.0f)
		{
			busys[lhs.peek()] = true;
			game::render::quad_set_flipbook(quads[lhs.peek()], creatures[lhs.peek()].idle);

			auto& victim_hp = hps[rhs.peek()];
			if(victim_hp-- <= 1)
			{
				impl_on_death(rhs);
			}
			else
			{
				// rhs got hurt
				if(rhs != player_get_avatar() && rhs != enemy_get_avatar())
				{
					entity_set_target(rhs, lhs);
				}
				if(healthbars[rhs.peek()] == tz::nullhand)
				{
					healthbars[rhs.peek()] = create_entity({.prefab_name = "healthbar", .position = tz::v2f::zero(), .parent = rhs});
					healthbar_timeouts[rhs.peek()] = config_healthbar_duration;
				}
			}
			// rhs should retaliate.
			cooldowns[lhs.peek()] = creatures[lhs.peek()].base_cooldown;
		}	
	}

	void impl_on_death(entity_handle ent)
	{
		auto prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_death; if fn ~= nil then fn({}) end", prefab.name, ent.peek())));
		if(player_targets(ent))
		{
			player_drop_target_entity();
		}
		if(ent == player_get_avatar())
		{
			player_on_death();
		}
		else if(ent == enemy_get_avatar())
		{
			// you won
		}
		else
		{
			destroy_entity(ent);
		}

		// everyone targetting it should drop target.
		iterate_entities([dead_person = ent](entity_handle ent)
		{
			if(
				ent == dead_person ||
				!creatures[ent.peek()].combat ||
				ent == enemy_get_avatar() ||
				ent == player_get_avatar()	
				) return;
			if(entity_get_target(ent) == dead_person)
			{
				busys[ent.peek()] = false;
				// an entity was targetting the now dead entity.
				// was it player aligned?
				if(player_aligneds[ent.peek()])
				{
					// yes, it should target the players target
					if(player_get_target() != tz::nullhand)
					{
						entity_set_target(ent, player_get_target());
					}
					else if(player_get_target_location().has_value())
					{
						entity_set_target_location(ent, player_get_target_location().value());
					}
					else if(enemy_get_avatar() != tz::nullhand && entity_get_hp(enemy_get_avatar()) > 0)
					{
						// player hasn't targetted anything, go for the enemy!
						entity_set_target(ent, enemy_get_avatar());
					}
				}
				else if(player_get_avatar() != tz::nullhand && entity_get_hp(player_get_avatar()) > 0)
				{
					// enemy aligned. it should go straight for the player
					entity_set_target(ent, player_get_avatar());
				}
				else
				{
					// nothing to go for.
					entity_set_target(ent, tz::nullhand);
				}
			}
		});
	}
}