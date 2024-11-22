prefabs.peasant =
{
	display_name = "Peasant",
	description = "bro has 0 money",
	on_update = function(ent, delta_seconds)
		ai.wander(ent)
	end,
	base_health = 10,
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
	}
}