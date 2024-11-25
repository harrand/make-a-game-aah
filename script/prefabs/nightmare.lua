prefabs.nightmare =
{
	display_name = "Melistra's Nightmare",
	base_health = 50,
	movement_speed = 1.3,
	base_cooldown = 0.3,
	power = 5,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/nightmare/idle0.png",
			"creature/nightmare/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/nightmare/idle0.png",
			"creature/nightmare/side0.png",
			"creature/nightmare/idle0.png",
			"creature/nightmare/side1.png"
		}
	},
	cast =
	{
		fps = 7,
		loop = true,
		frames =
		{
			"creature/nightmare/cast0.png",
			"creature/nightmare/cast1.png",
			"creature/nightmare/cast2.png",
			"creature/nightmare/cast3.png",
			"creature/nightmare/cast4.png",
			"creature/nightmare/cast5.png",
			"creature/nightmare/cast6.png"
		}
	}
}