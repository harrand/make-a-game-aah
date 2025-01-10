prefabs.leech_life =
{
	display_name = "Leech Life",
	description = "Launch a bolt of nature magic at a target, draining life into a second bolt that returns and heals you.",
	base_health = 99999,
	base_cooldown = 99999,
	base_damage = 5,
	movement_speed = 3,
	leeway_coefficient = 0.05,
	colour_tint_r = 1.0,
	colour_tint_g = 0.2,
	colour_tint_b = 0.2,
	initial_scale_x = 0.3,
	initial_scale_y = 0.3,
	emissive = true,
	face_move_direction = true,
	spell_decoration = true,
	attackable = false,
	require_target_entity_to_play = true,
	power = 2,
	idle =
	{
		fps = 12,
		loop = true,
		frames =
		{
			"spell/sparkle/0.png",
			"spell/sparkle/1.png",
			"spell/sparkle/2.png",
			"spell/sparkle/3.png",
			"spell/sparkle/4.png",
			"spell/sparkle/5.png",
			"spell/sparkle/6.png",
			"spell/sparkle/7.png",
		},
		emissive_frames =
		{
			"spell/sparkle/0.png",
			"spell/sparkle/1.png",
			"spell/sparkle/2.png",
			"spell/sparkle/3.png",
			"spell/sparkle/4.png",
			"spell/sparkle/5.png",
			"spell/sparkle/6.png",
			"spell/sparkle/7.png",
		}
	},
	on_update = function(me, delta)
		local tar = entity_get_target(me)
		local usrdata = entity_get_userdata(me)
		-- if tar is ever nil, we have dropped target, so we need to despawn
		-- when we are created, we expect our userdata to contain our initial target
		-- so if our target ever changes then we must despawn, or the leech_life will suddenly change trajectory.
		if tar == nil or (tar ~= usrdata) then
			destroy_entity(me)
		end
	end,
	on_create = function(me)
		entity_set_scale(me, 0.6, 0.3)
	end,
	on_hit = function(me, victim)
		local lifebolt = create_entity("lifebolt")
		local owner = entity_get_owner(me)
		entity_set_position(lifebolt, entity_get_position(me))
		entity_set_is_player_aligned(lifebolt, not entity_is_player_aligned(me))
		entity_set_owner(lifebolt, victim)
		entity_set_target(lifebolt, owner)
		entity_set_userdata(lifebolt, owner)

		destroy_entity(me)
	end
}
prefabs.leech_life.move_horizontal = prefabs.leech_life.idle
