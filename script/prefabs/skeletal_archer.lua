prefabs.skeletal_archer =
{
	display_name = "Skeletal Archer",
	description = "An undead ranged attacker.",
	base_health = 5,
	base_cooldown = 0.7,
	base_damage = 0,
	movement_speed = 0.5,
	power = 2,
	emissive = true,
	undead = true,
	leeway_coefficient = 15.0,
	on_hit = function(me, victim)
		local arrow = create_entity("arrow")
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
			"creature/skeletal_archer/idle0.png",
			"creature/skeletal_archer/idle1.png"
		},
		emissive_frames =
		{
			"creature/skeletal_archer/emissive_idle0.png",
			"creature/skeletal_archer/emissive_idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/skeletal_archer/idle0.png",
			"creature/skeletal_archer/side0.png",
			"creature/skeletal_archer/idle0.png",
			"creature/skeletal_archer/side1.png"
		},
		emissive_frames =
		{
			"creature/skeletal_archer/emissive_idle0.png",
			"creature/skeletal_archer/emissive_side0.png",
			"creature/skeletal_archer/emissive_idle0.png",
			"creature/skeletal_archer/emissive_side1.png"
		}
	},
	attack =
	{
		fps = 12,
		loop = true,
		frames =
		{
			"creature/skeletal_archer/idle0.png",
			"creature/skeletal_archer/attack0.png",
			"creature/skeletal_archer/attack0.png",
			"creature/skeletal_archer/attack0.png",
			"creature/skeletal_archer/attack1.png",
			"creature/skeletal_archer/attack1.png",
			"creature/skeletal_archer/attack1.png",
			"creature/skeletal_archer/attack2.png",
			"creature/skeletal_archer/attack2.png",
			"creature/skeletal_archer/attack2.png",
			"creature/skeletal_archer/idle0.png"
		},
		emissive_frames =
		{
			"creature/skeletal_archer/emissive_idle0.png",
			"creature/skeletal_archer/emissive_attack0.png",
			"creature/skeletal_archer/emissive_attack0.png",
			"creature/skeletal_archer/emissive_attack0.png",
			"creature/skeletal_archer/emissive_attack1.png",
			"creature/skeletal_archer/emissive_attack1.png",
			"creature/skeletal_archer/emissive_attack1.png",
			"creature/skeletal_archer/emissive_attack2.png",
			"creature/skeletal_archer/emissive_attack2.png",
			"creature/skeletal_archer/emissive_attack2.png",
			"creature/skeletal_archer/emissive_idle0.png"
		}
	}
}
