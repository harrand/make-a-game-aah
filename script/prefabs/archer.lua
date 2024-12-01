prefabs.archer =
{
	display_name = "Archer",
	description = "bro has 0 money",
	base_health = 5,
	base_cooldown = 0.7,
	movement_speed = 0.5,
	power = 3,
	leeway_coefficient = 8.0,
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