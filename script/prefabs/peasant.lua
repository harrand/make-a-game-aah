prefabs.peasant =
{
	display_name = "Peasant",
	description = "An angry man with a rusty pitchfork.",
	base_health = 5,
	movement_speed = 1.2,
	power = 1,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/peasant/idle0.png",
			"creature/peasant/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/peasant/idle0.png",
			"creature/peasant/side0.png",
			"creature/peasant/idle0.png",
			"creature/peasant/side1.png"
		}
	},
	attack =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/peasant/attack0.png",
			"creature/peasant/attack1.png"
		}
	}
}