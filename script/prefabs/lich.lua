prefabs.lich =
{
	display_name = "Lich",
	description = "A powerful wizard who commands the undead.",
	base_health = 40,
	base_cooldown = 1.6667,
	base_damage = 0,
	movement_speed = 0.4,
	power = 5,
	emissive = true,
	leeway_coefficient = 18.0,
	on_hit = function(me, victim)
		local shadowbolt = create_entity("shadowbolt")
		entity_set_is_player_aligned(shadowbolt, entity_is_player_aligned(me))
		entity_set_position(shadowbolt, entity_get_position(me))
		entity_set_target(shadowbolt, victim)
		entity_set_userdata(shadowbolt, victim)
		entity_set_owner(shadowbolt, me)
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/lich/idle0.png",
			"creature/lich/idle1.png"
		},
		emissive_frames =
		{
			"creature/lich/emissive_idle0.png",
			"creature/lich/emissive_idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 4,
		loop = true,
		frames =
		{
			"creature/lich/idle0.png",
			"creature/lich/side0.png",
			"creature/lich/idle0.png",
			"creature/lich/side1.png",
		},
		emissive_frames =
		{
			"creature/lich/emissive_idle0.png",
			"creature/lich/emissive_side0.png",
			"creature/lich/emissive_idle0.png",
			"creature/lich/emissive_side1.png",
		}
	},

	attack =
	{
		fps = 6,
		loop = true,
		frames =
		{
			"creature/lich/attack0.png",
			"creature/lich/attack1.png",
			"creature/lich/attack2.png",
			"creature/lich/attack3.png",
			"creature/lich/attack4.png",
			"creature/lich/attack5.png",
			"creature/lich/attack5.png",
			"creature/lich/attack6.png",
			"creature/lich/attack6.png",
			"creature/lich/attack7.png",
		},
		emissive_frames =
		{
			"creature/lich/emissive_attack0.png",
			"creature/lich/emissive_attack1.png",
			"creature/lich/emissive_attack2.png",
			"creature/lich/emissive_attack3.png",
			"creature/lich/emissive_attack4.png",
			"creature/lich/emissive_attack5.png",
			"creature/lich/emissive_attack5.png",
			"creature/lich/emissive_attack6.png",
			"creature/lich/emissive_attack6.png",
			"creature/lich/emissive_attack7.png",
		}
	}
}
