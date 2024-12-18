prefabs.general =
{
	display_name = "The General",
	description = "A mighty warrior who commands the Knights.",
	base_health = 80,
	movement_speed = 1.2,
	base_cooldown = 0.35,
	base_damage = 2,
	power = 5,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/general/idle0.png",
			"creature/general/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 7,
		loop = true,
		frames =
		{
			"creature/general/idle0.png",
			"creature/general/side0.png",
			"creature/general/idle0.png",
			"creature/general/side1.png"
		}
	},
	attack =
	{
		fps = 7,
		loop = true,
		frames =
		{
			"creature/general/attack0.png",
			"creature/general/attack1.png",
			"creature/general/attack0.png",
			"creature/general/attack2.png",
			"creature/general/attack2.png",
		}
	}
}
