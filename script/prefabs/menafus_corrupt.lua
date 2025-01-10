prefabs.menafus_corrupt =
{
	display_name = "Archmage Menafus",
	description = "You sense an evil force...",
	base_health = 100,
	base_cooldown = 1.0,
	base_damage = 0,
	movement_speed = 0.4,
	power = 5,
	undead = true,
	emissive = true,
	leeway_coefficient = 13.0,
	on_hit = function(me, victim)
		local firebolt = create_entity("firebolt")
		entity_set_is_player_aligned(firebolt, entity_is_player_aligned(me))
		entity_set_position(firebolt, entity_get_position(me))
		entity_set_target(firebolt, victim)
		entity_set_userdata(firebolt, victim)
		entity_set_owner(firebolt, me)
	end,
	on_death = function(me)
		local is_player_aligned = entity_is_player_aligned(me)
		local cleansed = create_entity("menafus")
		entity_set_position(cleansed, entity_get_position(me))
		entity_set_is_player_aligned(cleansed, not is_player_aligned)
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/menafus_corrupt/idle0.png",
			"creature/menafus_corrupt/idle1.png"
		},
		emissive_frames =
		{
			"creature/menafus_corrupt/idle0_emissive.png",
			"creature/menafus_corrupt/idle1_emissive.png"
		}
	},
	move_horizontal =
	{
		fps = 4,
		loop = true,
		frames =
		{
			"creature/menafus_corrupt/idle0.png",
			"creature/menafus_corrupt/side0.png",
			"creature/menafus_corrupt/idle0.png",
			"creature/menafus_corrupt/side1.png",
		},
		emissive_frames =
		{
			"creature/menafus_corrupt/idle0_emissive.png",
			"creature/menafus_corrupt/side0_emissive.png",
			"creature/menafus_corrupt/idle0_emissive.png",
			"creature/menafus_corrupt/side1_emissive.png",
		}
	},

	attack =
	{
		fps = 9,
		loop = true,
		frames =
		{
			"creature/menafus_corrupt/idle0.png",
			"creature/menafus_corrupt/attack0.png",
			"creature/menafus_corrupt/attack0.png",
			"creature/menafus_corrupt/attack0.png",
			"creature/menafus_corrupt/attack0.png",
			"creature/menafus_corrupt/attack1.png",
			"creature/menafus_corrupt/attack1.png",
			"creature/menafus_corrupt/attack1.png",
			"creature/menafus_corrupt/attack1.png",
		},
		emissive_frames =
		{
			"creature/menafus_corrupt/idle0_emissive.png",
			"creature/menafus_corrupt/attack0_emissive.png",
			"creature/menafus_corrupt/attack0_emissive.png",
			"creature/menafus_corrupt/attack0_emissive.png",
			"creature/menafus_corrupt/attack0_emissive.png",
			"creature/menafus_corrupt/attack1_emissive.png",
			"creature/menafus_corrupt/attack1_emissive.png",
			"creature/menafus_corrupt/attack1_emissive.png",
			"creature/menafus_corrupt/attack1_emissive.png",
		}
	}
}

