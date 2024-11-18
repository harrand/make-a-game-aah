prefabs.skeleton =
{
	on_create = function(ent)
		local child = create_entity("peasant")
		entity_set_parent(child, ent)
		entity_set_scale(child, 0.5, 0.5)
	end,
	base_health = 10,
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