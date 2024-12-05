prefabs.archer =
{
	display_name = "Archer",
	description = "A ranged attacker with a keen eye.",
	base_health = 5,
	base_cooldown = 0.7,
	base_damage = 0,
	movement_speed = 0.5,
	power = 3,
	leeway_coefficient = 8.0,
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
			"creature/archer/idle0.png",
			"creature/archer/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/archer/idle0.png",
			"creature/archer/side0.png",
			"creature/archer/idle0.png",
			"creature/archer/side1.png"
		}
	},
	attack =
	{
		fps = 12,
		loop = true,
		frames =
		{
			"creature/archer/idle0.png",
			"creature/archer/attack0.png",
			"creature/archer/attack0.png",
			"creature/archer/attack0.png",
			"creature/archer/attack1.png",
			"creature/archer/attack1.png",
			"creature/archer/attack1.png",
			"creature/archer/attack2.png",
			"creature/archer/attack2.png",
			"creature/archer/attack2.png",
			"creature/archer/idle0.png"
		}
	}
}
