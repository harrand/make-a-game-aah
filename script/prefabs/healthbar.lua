prefabs.healthbar =
{
	impl_scale = {0.5, 0.1},
	on_create = function(ent)
		local margin = 0.4
		entity_set_position(ent, 0, 0.15)
		entity_set_scale(ent, prefabs.healthbar.impl_scale[1], prefabs.healthbar.impl_scale[2])

		-- todo: uncomment this out when on_update knows how to affect this child instead of the entity itself.
		--local child = create_entity("empty")
		--entity_set_colour_tint(child, 0.0, 0.5, 0.0)
		--entity_set_parent(child, ent)
		--local diff = prefabs.healthbar.impl_scale[2] - (prefabs.healthbar.impl_scale[2] / (1.0 + margin))
		--entity_set_scale(child, prefabs.healthbar.impl_scale[1] - diff, prefabs.healthbar.impl_scale[2] / (1.0 + margin))
	end,
	on_update = function(ent, delta_seconds)
		-- todo: have this affect the bar child instead of the entity itself. see the end of on_create for context.

		-- max hp = {0.0, 0.5, 0.0}
		-- 0   hp = {1.0, 0.0, 0.0}
		local hp_pct = entity_get_hp(ent) / entity_get_max_hp(ent)
		entity_set_colour_tint(ent, 1.0 - hp_pct, 0.5 * hp_pct, 0.0)
		entity_set_scale(ent, hp_pct * prefabs.healthbar.impl_scale[1], prefabs.healthbar.impl_scale[2])
	end,
	base_health = 1,
	movement_speed = 0,
	power = 1
}