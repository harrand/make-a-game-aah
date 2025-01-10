prefabs.menafus =
{
	display_name = "Archmage Menafus",
	base_health = 100,
	base_cooldown = 1.0,
	base_damage = 0,
	movement_speed = 0.4,
	power = 5,
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
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/menafus/idle0.png",
			"creature/menafus/idle1.png"
		},
		emissive_frames =
		{
			"creature/menafus/idle0_emissive.png",
			"creature/menafus/idle1_emissive.png"
		}
	},
	move_horizontal =
	{
		fps = 4,
		loop = true,
		frames =
		{
			"creature/menafus/idle0.png",
			"creature/menafus/side0.png",
			"creature/menafus/idle0.png",
			"creature/menafus/side1.png",
		},
		emissive_frames =
		{
			"creature/menafus/idle0_emissive.png",
			"creature/menafus/side0_emissive.png",
			"creature/menafus/idle0_emissive.png",
			"creature/menafus/side1_emissive.png",
		}
	},

	attack =
	{
		fps = 9,
		loop = true,
		frames =
		{
			"creature/menafus/idle0.png",
			"creature/menafus/attack0.png",
			"creature/menafus/attack0.png",
			"creature/menafus/attack0.png",
			"creature/menafus/attack0.png",
			"creature/menafus/attack1.png",
			"creature/menafus/attack1.png",
			"creature/menafus/attack1.png",
			"creature/menafus/attack1.png",
		},
		emissive_frames =
		{
			"creature/menafus/idle0_emissive.png",
			"creature/menafus/attack0_emissive.png",
			"creature/menafus/attack0_emissive.png",
			"creature/menafus/attack0_emissive.png",
			"creature/menafus/attack0_emissive.png",
			"creature/menafus/attack1_emissive.png",
			"creature/menafus/attack1_emissive.png",
			"creature/menafus/attack1_emissive.png",
			"creature/menafus/attack1_emissive.png",
		}
	}
}
