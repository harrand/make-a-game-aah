prefabs.skeleton =
{
	display_name = "Skeleton",
	description = "Spooky and scary!",
	base_health = 5,
	base_cooldown = 0.75,
	movement_speed = 1.0,
	power = 2,
	emissive = true,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/skeleton/idle0.png",
			"creature/skeleton/idle1.png"
		},
		emissive_frames =
		{
			"creature/skeleton/emissive_idle0.png",
			"creature/skeleton/emissive_idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/skeleton/idle0.png",
			"creature/skeleton/side0.png",
			"creature/skeleton/idle0.png",
			"creature/skeleton/side1.png"
		},
		emissive_frames =
		{
			"creature/skeleton/emissive_idle0.png",
			"creature/skeleton/emissive_side0.png",
			"creature/skeleton/emissive_idle0.png",
			"creature/skeleton/emissive_side1.png"
		}
	},
	attack =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/skeleton/idle0.png",
			"creature/skeleton/attack0.png",
			"creature/skeleton/attack1.png",
			"creature/skeleton/attack1.png",
			"creature/skeleton/attack2.png",
			"creature/skeleton/attack3.png",
		},
		emissive_frames =
		{
			"creature/skeleton/emissive_idle0.png",
			"creature/skeleton/emissive_attack0.png",
			"creature/skeleton/emissive_attack1.png",
			"creature/skeleton/emissive_attack1.png",
			"creature/skeleton/emissive_attack2.png",
			"creature/skeleton/emissive_attack3.png",
		}
	}
}
