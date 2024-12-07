prefabs.assassin =
{
	display_name = "Assassin",
	description = "A deadly, but fragile attacker who excels at picking off lone enemies.",
	base_health = 6,
	base_cooldown = 0.2,
	movement_speed = 2.2,
	leeway_coefficient = 0.1,
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
