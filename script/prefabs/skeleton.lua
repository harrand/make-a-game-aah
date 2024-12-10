prefabs.skeletal_warrior =
{
	display_name = "Skeletal Warrior",
	description = "A durable undead warrior. Taunt.",
	base_health = 27,
	base_cooldown = 0.75,
	movement_speed = 1.0,
	power = 2,
	emissive = true,
	undead = true,
	taunt = true,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/skeletal_warrior/idle0.png",
			"creature/skeletal_warrior/idle1.png"
		},
		emissive_frames =
		{
			"creature/skeletal_warrior/emissive_idle0.png",
			"creature/skeletal_warrior/emissive_idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/skeletal_warrior/idle0.png",
			"creature/skeletal_warrior/side0.png",
			"creature/skeletal_warrior/idle0.png",
			"creature/skeletal_warrior/side1.png"
		},
		emissive_frames =
		{
			"creature/skeletal_warrior/emissive_idle0.png",
			"creature/skeletal_warrior/emissive_side0.png",
			"creature/skeletal_warrior/emissive_idle0.png",
			"creature/skeletal_warrior/emissive_side1.png"
		}
	},
	attack =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/skeletal_warrior/idle0.png",
			"creature/skeletal_warrior/attack0.png",
			"creature/skeletal_warrior/attack1.png",
			"creature/skeletal_warrior/attack1.png",
			"creature/skeletal_warrior/attack2.png",
			"creature/skeletal_warrior/attack3.png",
		},
		emissive_frames =
		{
			"creature/skeletal_warrior/emissive_idle0.png",
			"creature/skeletal_warrior/emissive_attack0.png",
			"creature/skeletal_warrior/emissive_attack1.png",
			"creature/skeletal_warrior/emissive_attack1.png",
			"creature/skeletal_warrior/emissive_attack2.png",
			"creature/skeletal_warrior/emissive_attack3.png",
		}
	}
}
