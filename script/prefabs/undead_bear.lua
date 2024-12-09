prefabs.undead_bear =
{
	display_name = "Undead Bear",
	description = "A bear's corpse, cruelly raised from the dead",
	base_health = 50,
	movement_speed = 0.5,
	base_cooldown = 2.0,
	base_damage = 3,
	taunt = true,
	emissive = true,
	power = 4,
	idle =
	{
		fps = 1,
		loop = true,
		frames =
		{
			"creature/undead_bear/idle0.png",
			"creature/undead_bear/idle1.png"
		},
		emissive_frames =
		{
			"creature/undead_bear/emissive_idle0.png",
			"creature/undead_bear/emissive_idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/undead_bear/idle0.png",
			"creature/undead_bear/side0.png",
			"creature/undead_bear/idle0.png",
			"creature/undead_bear/side1.png",
		},
		emissive_frames =
		{
			"creature/undead_bear/emissive_idle0.png",
			"creature/undead_bear/emissive_side0.png",
			"creature/undead_bear/emissive_idle0.png",
			"creature/undead_bear/emissive_side1.png",
		}
	},
	attack =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/undead_bear/attack0.png",
			"creature/undead_bear/attack1.png",
		},
		emissive_frames =
		{
			"creature/undead_bear/emissive_attack0.png",
			"creature/undead_bear/emissive_attack1.png",
		}
	}
}
