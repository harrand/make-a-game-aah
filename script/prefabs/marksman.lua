prefabs.marksman =
{
	display_name = "Marksman",
	description = "A deadly ranged attacker.",
	base_health = 15,
	base_cooldown = 3.0,
	base_damage = 0,
	movement_speed = 0.8,
	power = 3,
	emissive = true,
	leeway_coefficient = 15.0,
	undead_variant = "skeletal_archer",
	on_hit = function(me, victim)
		local arrow = create_entity("burning_arrow")
		entity_set_is_player_aligned(arrow, entity_is_player_aligned(me))
		entity_set_position(arrow, entity_get_position(me))
		entity_set_target(arrow, victim)
		entity_set_userdata(arrow, victim)
		entity_set_owner(arrow, me)
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/marksman/idle0.png",
			"creature/marksman/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/marksman/idle0.png",
			"creature/marksman/side0.png",
			"creature/marksman/idle0.png",
			"creature/marksman/side1.png"
		}
	},
	attack =
	{
		fps = 7.5,
		loop = true,
		frames =
		{
			"creature/marksman/idle0.png",
			"creature/marksman/attack0.png",
			"creature/marksman/attack0.png",
			"creature/marksman/attack0.png",
			"creature/marksman/attack1.png",
			"creature/marksman/attack1.png",
			"creature/marksman/attack1.png",
			"creature/marksman/attack2.png",
			"creature/marksman/attack2.png",
			"creature/marksman/attack2.png",
		},
		emissive_frames =
		{
			"creature/marksman/attack0_emissive.png",
			"creature/marksman/attack0_emissive.png",
			"creature/marksman/attack0_emissive.png",
			"creature/marksman/attack0_emissive.png",
			"creature/marksman/attack1_emissive.png",
			"creature/marksman/attack1_emissive.png",
			"creature/marksman/attack1_emissive.png",
			"creature/marksman/attack2_emissive.png",
			"creature/marksman/attack2_emissive.png",
			"creature/marksman/attack2_emissive.png",
		}
	}
}
