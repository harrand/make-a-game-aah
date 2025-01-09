#include "entity.hpp"
#include "render.hpp"
#include "prefab.hpp"
#include "config.hpp"
#include "player.hpp"
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
	std::vector<unsigned int> damages = {};
	std::vector<float> leeway_coefficients = {};
	std::vector<entity_handle> parents = {};
	std::vector<entity_handle> owners = {};
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
	void impl_on_kill(entity_handle ent, entity_handle victim);
	bool impl_entity_destroyed(entity_handle ent);
	void impl_all_stop_targetting(entity_handle ent);

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
			damages.push_back(1);
			leeway_coefficients.push_back(1.0f);
			parents.push_back({});
			owners.push_back({});
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

		speeds[ret.peek()] = prefab.movement_speed;
		hps[ret.peek()] = info.hp == -1u ? prefab.base_health : info.hp;

		player_aligneds[ret.peek()] = info.player_aligned;
		positions[ret.peek()] = info.position;
		rotations[ret.peek()] = info.rotation;
		scales[ret.peek()] = info.scale;
		cooldowns[ret.peek()] = 0.0f;
		damages[ret.peek()] = prefab.base_damage;
		leeway_coefficients[ret.peek()] = prefab.leeway_coefficient;
		parents[ret.peek()] = tz::nullhand;
		owners[ret.peek()] = tz::nullhand;
		userdatas[ret.peek()] = info.userdata;
		patrols[ret.peek()] = {};
		patrol_cursors[ret.peek()] = 0;

		quads[ret.peek()] = game::render::create_quad({.position = info.position, .rotation = info.rotation, .scale = info.scale, .colour = prefab.colour_tint},
				prefab.emissive ? render::quad_flag::emissive : static_cast<render::quad_flag>(0));
		game::render::quad_set_flipbook(quads[ret.peek()], prefab.idle);
		move_dirs[ret.peek()] = tz::v2f::zero();
		target_locations[ret.peek()] = std::nullopt;
		targets[ret.peek()] = tz::nullhand;
		childrens[ret.peek()] = {};
		tz_assert(tooltips[ret.peek()] == tz::nullhand, "tooltip not cleared when creating new entity");
		tooltips[ret.peek()] = tz::nullhand;
		tz_assert(healthbars[ret.peek()] == tz::nullhand, "tooltip not cleared when creating new entity");
		healthbars[ret.peek()] = tz::nullhand;
		healthbar_timeouts[ret.peek()] = 0.0f;
		busys[ret.peek()] = false;

		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_create; if fn ~= nil then fn({}) end", prefab.name, ret.peek())));
		if(info.parent != tz::nullhand)
		{
			entity_set_parent(ret, info.parent);
		}
		return ret;
	}

	void destroy_entity(entity_handle ent)
	{
		auto& prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_destroy; if fn ~= nil then fn({}) end", prefab.name, ent.peek())));
		entity_set_parent(ent, tz::nullhand);

		auto children = childrens[ent.peek()];
		for(auto child : children)
		{
			entity_set_parent(child, tz::nullhand);
		}

		auto free_list_iter = std::find(entity_free_list.begin(), entity_free_list.end(), ent);
		if(free_list_iter != entity_free_list.end())
		{
			tz_error("double destroy on entity {} ({})", ent.peek(), creatures[ent.peek()].name);
		}

		entity_free_list.push_back(ent);
		impl_all_stop_targetting(ent);
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
			const float default_speed = entity_get_prefab(ent).movement_speed;
			const float leeway = config_global_speed_multiplier * default_speed * config_global_leeway_dist * leeway_coefficients[i];
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
			if(maybe_tarloc.has_value() && speeds[ent.peek()] > 0.0f)
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

					if(creatures[i].face_move_direction)
					{
						float rotation = std::atan2(move_dir[1], move_dir[0]);
						if(move_dir[0] < 0.0f)
						{
							rotation += 3.14159f;
						}
						render::quad_set_rotation(quads[i], rotation);
					}

				}
				else if(!busys[i])
				{
					if(creatures[i].combat)
					{
						player_handle p = game::try_get_player_that_controls_entity(ent);
						if(p != tz::nullhand)
						{
							game::player_control_entity(p, ent);
						}
					}
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

	game::prefab entity_get_prefab(entity_handle ent)
	{
		return creatures[ent.peek()];
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

	float entity_get_movement_speed(entity_handle ent)
	{
		return speeds[ent.peek()];
	}

	void entity_set_movement_speed(entity_handle ent, float movement_speed)
	{
		speeds[ent.peek()] = movement_speed;
	}

	void entity_set_cooldown(entity_handle ent, float cooldown)
	{
		cooldowns[ent.peek()] = cooldown;
	}

	entity_handle entity_get_owner(entity_handle ent)
	{
		return owners[ent.peek()];
	}

	void entity_set_owner(entity_handle ent, entity_handle owner)
	{
		owners[ent.peek()] = owner;
	}

	bool entity_is_player_aligned(entity_handle ent)
	{
		return player_aligneds[ent.peek()];
	}

	void entity_set_is_player_aligned(entity_handle ent, bool player_aligned)
	{
		bool aligned = player_aligneds[ent.peek()];
		// clear target if swapping sides
		player_aligneds[ent.peek()] = player_aligned;
		game::iterate_players([ent, good = aligned](player_handle p)
		{
			if(player_targets(p, ent) && good != player_is_good(p))
			{
				player_drop_target_entity(p);
			}
		});

		if(aligned != player_aligned)
		{
			entity_set_target(ent, tz::nullhand);
			impl_all_stop_targetting(ent);
		}
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
		if(targets[ent.peek()] != tz::nullhand)
		{
			// if we already have a target, cancel this operation if its a taunt entity
			// unless of course you want to set target to null which case we do respect that.
			if(creatures[targets[ent.peek()].peek()].taunt && tar != tz::nullhand)
			{
				return;
			}
		}
		targets[ent.peek()] = tar;
		// dropping a target will stop you from being busy.
		if(tar == tz::nullhand)
		{
			busys[ent.peek()] = false;
		}
		else
		{
			float ent_x = entity_get_position(ent)[0];
			float tar_x = entity_get_position(tar)[0];
			if(ent_x > tar_x)
			{
				entity_face_left(ent);
			}
			else
			{
				entity_face_right(ent);
			}
		}
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
		if(cooldowns[lhs.peek()] <= 0.0f && hps[lhs.peek()] > 0.0f && hps[rhs.peek()] > 0.0f)
		{
			busys[lhs.peek()] = true;

			auto flipbook = creatures[lhs.peek()].attack;
			if(flipbook == tz::nullhand)
			{
				flipbook = creatures[lhs.peek()].idle;
			}
			game::render::quad_set_flipbook(quads[lhs.peek()], flipbook);

			auto& victim_hp = hps[rhs.peek()];
			tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_hit; if fn ~= nil then _tmpret = fn({}, {}); end if _tmpret == nil then _tmpret = false end", creatures[lhs.peek()].name, lhs.peek(), rhs.peek())));
			bool cancel = tz_must(tz::lua_get_bool("_tmpret"));
			if(cancel)
			{
				tz_must(tz::lua_execute("_tmpret = false"));
				busys[lhs.peek()] = false;
				return;
			}
			// its possible on_hit leads to either entity being destroyed.
			if(!impl_entity_destroyed(rhs))
			{
				unsigned int dmg = damages[lhs.peek()];
				if(victim_hp <= dmg)
				{
					victim_hp = 0;

					entity_handle retaliation_target = lhs;
					while(owners[retaliation_target.peek()] != tz::nullhand && !creatures[retaliation_target.peek()].attackable)
					{
						retaliation_target = owners[retaliation_target.peek()];
					}
					if(!impl_entity_destroyed(retaliation_target))
					{
						impl_on_kill(retaliation_target, rhs);
					}
					impl_on_death(rhs);
				}
				else if(dmg > 0)
				{
					victim_hp -= dmg;
					// rhs got hurt

					// so by default we should have the victim target the attacker.
					// but what if the victim is struck by an arrow? they shouldnt hit the arrow, they should go for its owner
					// however, if you always target the owner, then if a knight attacks you, the retaliation target will be its owner i.e beeline straight for the player who summoned that knight.
					//
					entity_handle retaliation_target = lhs;
					while(owners[retaliation_target.peek()] != tz::nullhand && !creatures[retaliation_target.peek()].attackable)
					{
						retaliation_target = owners[retaliation_target.peek()];
					}
					if(!impl_entity_destroyed(retaliation_target))
					{
						entity_set_target(rhs, retaliation_target);
					}
					if(healthbars[rhs.peek()] == tz::nullhand)
					{
						healthbars[rhs.peek()] = create_entity({.prefab_name = "healthbar", .position = tz::v2f::zero(), .parent = rhs});
						healthbar_timeouts[rhs.peek()] = config_healthbar_duration;
					}
				}
			}
			if(!impl_entity_destroyed(lhs))
			{
				cooldowns[lhs.peek()] = creatures[lhs.peek()].base_cooldown;
			}
		}	
	}

	void impl_on_death(entity_handle ent)
	{
		auto prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_death; if fn ~= nil then fn({}) end", prefab.name, ent.peek())));
		bool ent_is_player = false;
		iterate_players([ent, &ent_is_player](player_handle p)
		{
			// all players must stop targetting it.
			if(player_targets(p, ent))
			{
				player_drop_target_entity(p);
			}
			// if it was a player, then invoke their callback.
			if(ent == player_get_avatar(p))
			{
				player_on_death(p);
				ent_is_player = true;
			}
		});
		if(!ent_is_player)
		{
			destroy_entity(ent);
		}
	}

	void impl_on_kill(entity_handle ent, entity_handle victim)
	{
		auto prefab = creatures[ent.peek()];
		tz_must(tz::lua_execute(std::format("local fn = prefabs.{}.on_kill; if fn ~= nil then fn({}, {}) end", prefab.name, ent.peek(), victim.peek())));
	}

	bool impl_entity_destroyed(entity_handle ent)
	{
		return std::find(entity_free_list.begin(), entity_free_list.end(), ent) != entity_free_list.end();
	}

	void impl_all_stop_targetting(entity_handle ent)
	{
		// everyone targetting it should drop target.
		iterate_players([ent](player_handle p)
		{
			if(player_targets(p, ent))
			{
				player_drop_target_entity(p);
			}
		});
		iterate_entities([dead_person = ent](entity_handle ent)
		{
			if(
				ent == dead_person ||
				!creatures[ent.peek()].combat
				) return;
			if(entity_get_target(ent) == dead_person)
			{
				busys[ent.peek()] = false;
				// an entity was targetting the now dead entity.
				// was it player aligned?
				// ok, then the player who is controlling it should now decide what it should do next.
				player_handle p = game::try_get_player_that_controls_entity(ent);
				volatile entity_handle not_this_one_please = dead_person;
				entity_set_target(ent, tz::nullhand);
				if(p != tz::nullhand)
				{
					game::player_control_entity(p, ent);
					tz_assert(entity_get_target(ent) != dead_person, "player made entity target a previous target instantly (said target destroyed: {})", impl_entity_destroyed(dead_person));
				}
			}
		});
	}
}
