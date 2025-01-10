prefabs.templar =
{
	display_name = "Templar",
	description = "A righteous warrior. Highly effective against undead enemies.",
	base_health = 20,
	base_cooldown = 0.75,
	base_damage = 1,
	movement_speed = 0.6,
	power = 3,
	taunt = true,
	on_hit = function(me, victim)
		-- everytime we hit an undead, leech their life.
		local victim_prefab = entity_get_prefab(victim)
		if prefabs[victim_prefab].undead then
			local leech = create_entity("leech_life")
			entity_set_position(leech, entity_get_position(me))
			entity_set_is_player_aligned(leech, entity_is_player_aligned(me))
			entity_set_owner(leech, me)
			entity_set_target(leech, victim)
			entity_set_userdata(leech, victim)
		end
	end,
	on_kill = function(me, victim)
		-- heal our owner
		local owner = entity_get_owner(me)
		if owner ~= nil then
			local enemy_owner = entity_get_owner(victim)
			if enemy_owner ~= nil then
				local lifebolt = create_entity("lifebolt")
				entity_set_position(lifebolt, entity_get_position(me))
				entity_set_is_player_aligned(lifebolt, not entity_is_player_aligned(me))
				entity_set_owner(lifebolt, enemy_owner)
				entity_set_target(lifebolt, owner)
				entity_set_userdata(lifebolt, owner)
			end
		end
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/templar/idle0.png",
			"creature/templar/idle1.png"
		},
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/templar/idle0.png",
			"creature/templar/side0.png",
			"creature/templar/idle0.png",
			"creature/templar/side1.png"
		},
	},
	attack =
	{
		fps = 8,
		loop = true,
		frames =
		{
			"creature/templar/idle0.png",
			"creature/templar/attack0.png",
			"creature/templar/attack1.png",
			"creature/templar/attack1.png",
			"creature/templar/attack2.png",
			"creature/templar/attack3.png",
		},
	}
}
