prefabs.skeleton =
{
	display_name = "Skeleton",
	description = "Spooky and scary!",
	base_health = 2,
	base_cooldown = 0.5,
	movement_speed = 1.0,
	power = 2,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/skeleton/idle0.png",
			"creature/skeleton/idle1.png"
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
		}
	}
}