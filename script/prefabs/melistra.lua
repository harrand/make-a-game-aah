prefabs.melistra =
{
	display_name = "Melistra",
	base_health = 40,
	base_cooldown = 0.75,
	base_damage = 0,
	movement_speed = 0.4,
	power = 5,
	emissive = true,
	leeway_coefficient = 13.0,
	on_hit = function(me, victim)
		local firebolt = create_entity("firebolt")
		entity_set_is_player_aligned(firebolt, entity_is_player_aligned(me))
		entity_set_position(firebolt, entity_get_position(me))
		entity_set_target(firebolt, victim)
		entity_set_userdata(firebolt, victim)
		entity_set_owner(firebolt, me)
	end,
	idle =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/melistra/idle0.png",
			"creature/melistra/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 4,
		loop = true,
		frames =
		{
			"creature/melistra/idle0.png",
			"creature/melistra/side0.png",
			"creature/melistra/idle0.png",
			"creature/melistra/side1.png",
		}
	},

	attack =
	{
		fps = 12,
		loop = true,
		frames =
		{
			"creature/melistra/attack0.png",
			"creature/melistra/attack1.png",
			"creature/melistra/attack2.png",
			"creature/melistra/attack3.png",
			"creature/melistra/attack4.png",
			"creature/melistra/attack5.png",
			"creature/melistra/attack5.png",
			"creature/melistra/attack6.png",
			"creature/melistra/attack6.png",
			"creature/melistra/attack7.png",
		},
		emissive_frames =
		{
			"creature/melistra/emissive_attack0.png",
			"creature/melistra/emissive_attack1.png",
			"creature/melistra/emissive_attack2.png",
			"creature/melistra/emissive_attack3.png",
			"creature/melistra/emissive_attack4.png",
			"creature/melistra/emissive_attack5.png",
			"creature/melistra/emissive_attack5.png",
			"creature/melistra/emissive_attack6.png",
			"creature/melistra/emissive_attack6.png",
			"creature/melistra/emissive_attack7.png",
		}
	}
}
