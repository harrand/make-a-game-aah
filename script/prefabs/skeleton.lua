prefabs.skeleton =
{
	display_name = "Skeleton",
	description = "Spooky and scary!",
	on_create = function(ent)
		local healthbar = create_entity("healthbar")
		entity_set_parent(healthbar, ent)
		entity_set_hp(ent, 1)

		local x, y = entity_get_position(ent)
		local patrol =
		{
			x - 1.0, y + 0.0,
			x + 1.0, y + 0.0
		}
		entity_set_patrol(ent, table.unpack(patrol))
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