prefabs.banshee =
{
	display_name = "Banshee",
	description = "aah! a banshee!!!! fuck!!",
	base_health = 10,
	base_cooldown = 0.7,
	movement_speed = 0.4,
	power = 3,
	on_hit = function(me, victim)
		entity_set_is_player_aligned(victim, entity_is_player_aligned(me))
		destroy_entity(me)

		local r, g, b = entity_get_colour_tint(victim)
		entity_set_colour_tint(victim, r * 0.4, g * 0.7, b * 1.5)
		return true
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/banshee/idle0.png",
			"creature/banshee/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/banshee/idle0.png",
			"creature/banshee/idle1.png"
		}
	},
}