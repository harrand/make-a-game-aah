prefabs.bear =
{
	display_name = "Bear",
	description = "A massive brown bear",
	base_health = 50,
	movement_speed = 0.5,
	base_cooldown = 2.0,
	base_damage = 3,
	taunt = true,
	power = 4,
	idle =
	{
		fps = 1,
		loop = true,
		frames =
		{
			"creature/bear/idle0.png",
			"creature/bear/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/bear/idle0.png",
			"creature/bear/side0.png",
			"creature/bear/idle0.png",
			"creature/bear/side1.png",
		}
	},
	attack =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/bear/attack0.png",
			"creature/bear/attack1.png",
		}
	}
}
