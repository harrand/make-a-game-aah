prefabs.banshee =
{
	display_name = "Banshee",
	description = "The wrathful ghost of a wronged woman. Will attempt to possess a victim.",
	base_health = 10,
	base_cooldown = 0.7,
	movement_speed = 0.4,
	power = 3,
	undead = true,
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
