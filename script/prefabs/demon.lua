prefabs.demon =
{
	display_name = "In'fesh Ja",
	base_health = 250,
	base_cooldown = 1.17,
	base_damage = 0,
	movement_speed = 1.2,
	power = 5,
	emissive = true,
	undead = true,
	initial_scale_x = 2.0,
	initial_scale_y = 2.0,
	leeway_coefficient = 6.0,
	on_hit = function(me, victim)
		local firebolt = create_entity("shadowbolt")
		entity_set_is_player_aligned(firebolt, entity_is_player_aligned(me))
		entity_set_position(firebolt, entity_get_position(me))
		entity_set_target(firebolt, victim)
		entity_set_userdata(firebolt, victim)
		entity_set_owner(firebolt, me)
	end,
	on_kill = function(me, victim)
		-- spawn a red-hued copy of the victim, but aligned with the demon
		local victim_prefab = entity_get_prefab(victim)
		local thrall = create_entity(victim_prefab)
		entity_set_position(thrall, entity_get_position(victim))
		entity_set_is_player_aligned(thrall, entity_is_player_aligned(me))
		entity_set_owner(thrall, me)
		entity_set_colour_tint(thrall, 0.5, 0.2, 0.2)
		local new_name = "Servant of " .. prefabs.demon.display_name
		entity_set_display_name(thrall, new_name)

	end,
	idle =
	{
		fps = 6,
		loop = true,
		frames =
		{
			"creature/demon/idle0.png",
			"creature/demon/idle1.png",
			"creature/demon/idle0.png",
			"creature/demon/idle2.png",
		},
		emissive_frames =
		{
			"creature/demon/idle0_emissive.png",
			"creature/demon/idle1_emissive.png",
			"creature/demon/idle0_emissive.png",
			"creature/demon/idle2_emissive.png",
		}
	},
	attack =
	{
		fps = 6,
		loop = true,
		frames =
		{
			"creature/demon/attack0.png",
			"creature/demon/attack1.png",
			"creature/demon/attack2.png",
			"creature/demon/attack3.png",
			"creature/demon/attack4.png",
			"creature/demon/attack5.png",
			"creature/demon/attack4.png",
		},
		emissive_frames =
		{
			"creature/demon/attack0_emissive.png",
			"creature/demon/attack1_emissive.png",
			"creature/demon/attack2_emissive.png",
			"creature/demon/attack3_emissive.png",
			"creature/demon/attack4_emissive.png",
			"creature/demon/attack5_emissive.png",
			"creature/demon/attack4_emissive.png",
		},
	}
}
prefabs.demon.move_horizontal = prefabs.demon.idle
