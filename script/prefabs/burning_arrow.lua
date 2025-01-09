prefabs.burning_arrow =
{
	display_name = "burning arrow",
	description = "Launch a bolt of fire at a target, dealing damage",
	base_health = 99999,
	base_cooldown = 99999,
	base_damage = 4,
	movement_speed = 2.5,
	leeway_coefficient = 0.05,
	face_move_direction = true,
	spell_decoration = true,
	emissive = true,
	attackable = false,
	require_target_entity_to_play = true,
	power = 2,
	idle =
	{
		fps = 1,
		loop = true,
		frames =
		{
			"spell/burning_arrow.png"
		},
		emissive_frames =
		{
			"spell/burning_arrow_emissive.png"
		}
	},
	on_update = function(me, delta)
		local tar = entity_get_target(me)
		local usrdata = entity_get_userdata(me)
		-- if tar is ever nil, we have dropped target, so we need to despawn
		-- when we are created, we expect our userdata to contain our initial target
		-- so if our target ever changes then we must despawn, or the arrow will suddenly change trajectory.
		if tar == nil or (tar ~= usrdata) then
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
prefabs.burning_arrow.move_horizontal = prefabs.burning_arrow.idle
