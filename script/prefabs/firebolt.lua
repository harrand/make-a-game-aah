prefabs.firebolt =
{
	display_name = "Firebolt",
	description = "Launch a bolt of fire at a target, dealing damage",
	base_health = 99999,
	base_cooldown = 99999,
	movement_speed = 2.5,
	leeway_coefficient = 0.05,
	face_move_direction = true,
	colour_tint_r = 1.0,
	colour_tint_g = 0.7,
	colour_tint_b = 0.2,
	spell_decoration = true,
	power = 2,
	idle =
	{
		fps = 12,
		loop = true,
		frames =
		{
			"spell/sparkle/0.png",
			"spell/sparkle/1.png",
			"spell/sparkle/2.png",
			"spell/sparkle/3.png",
			"spell/sparkle/4.png",
			"spell/sparkle/5.png",
			"spell/sparkle/6.png",
			"spell/sparkle/7.png",
		}
	},
}
prefabs.firebolt.move_horizontal = prefabs.firebolt.idle