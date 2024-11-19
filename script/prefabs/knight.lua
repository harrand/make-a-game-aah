prefabs.knight =
{
	on_update = function(ent, delta_seconds)
		ai.wander(ent)
	end,
	base_health = 90,
	movement_speed = 0.6,
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
			"creature/knight/side1.png"
		}
	}
}