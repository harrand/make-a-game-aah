prefabs.knight =
{
	display_name = "Knight",
	description = "An armoured warrior boasting a mighty greatsword.",
	base_health = 20,
	movement_speed = 0.6,
	base_cooldown = 0.5,
	power = 4,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/knight/idle0.png",
			"creature/knight/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/knight/idle0.png",
			"creature/knight/side0.png",
			"creature/knight/idle0.png",
			"creature/knight/side1.png",
		}
	},
	attack =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/knight/attack0.png",
			"creature/knight/attack1.png",
			"creature/knight/attack0.png",
			"creature/knight/attack2.png",
			"creature/knight/attack2.png",
		}
	}
}