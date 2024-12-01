prefabs.arrow =
{
	display_name = "arrow",
	description = "Launch a bolt of fire at a target, dealing damage",
	base_health = 99999,
	base_cooldown = 99999,
	movement_speed = 2.5,
	leeway_coefficient = 0.05,
	face_move_direction = true,
	spell_decoration = true,
	require_target_entity_to_play = true,
	power = 2,
	idle =
	{
		fps = 1,
		loop = true,
		frames =
		{
			"spell/arrow.png"
		}
	},
	on_update = function(me, delta)
		if entity_get_target(me) == nil then
			destroy_entity(me)
		end
	end,
	on_create = function(me)
		entity_set_scale(me, 0.3, 0.15)
	end,
	on_hit = function(me, victim)
		destroy_entity(me)
	end
}
prefabs.arrow.move_horizontal = prefabs.arrow.idle