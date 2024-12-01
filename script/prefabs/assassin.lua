prefabs.assassin =
{
	display_name = "assassin",
	description = "bro has 0 money",
	base_health = 8,
	base_cooldown = 0.2,
	movement_speed = 1.3,
	power = 4,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/assassin/idle0.png",
			"creature/assassin/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/assassin/idle0.png",
			"creature/assassin/side0.png",
			"creature/assassin/idle0.png",
			"creature/assassin/side1.png"
		}
	},
	attack =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/assassin/attack0.png",
			"creature/assassin/attack1.png"
		}
	}
}