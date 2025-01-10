prefabs.shadowbolt =
{
	display_name = "Shadowbolt",
	description = "Launch a bolt of darkness at a target, dealing high damage",
	base_health = 99999,
	base_cooldown = 99999,
	base_damage = 6,
	movement_speed = 2,
	leeway_coefficient = 0.05,
	colour_tint_r = 0.42,
	colour_tint_g = 0.0,
	colour_tint_b = 1.0,
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
		-- so if our target ever changes then we must despawn, or the shadowbolt will suddenly change trajectory.
		if tar == nil or (tar ~= usrdata) then
			destroy_entity(me)
		end
	end,
	on_hit = function(me, victim)
		destroy_entity(me)
	end,
}
prefabs.shadowbolt.move_horizontal = prefabs.shadowbolt.idle
