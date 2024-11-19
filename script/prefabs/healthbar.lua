prefabs.healthbar =
{
	impl_scale = {0.5, 0.1},
	impl_bar_margin = 0.6,
	on_create = function(ent)
		entity_set_scale(ent, prefabs.healthbar.impl_scale[1], prefabs.healthbar.impl_scale[2])
		entity_set_colour_tint(ent, 1.0, 1.0, 1.0)

		local child = create_entity("empty")
		entity_set_colour_tint(child, 0.0, 0.5, 0.0)
		entity_set_parent(child, ent)
		entity_set_userdata(ent, child)
	end,
	on_update = function(ent, delta_seconds)
		-- remember: userdata is the child bar
		-- parent is the thing we're monitoring
		local parent = entity_get_parent(ent)
		-- if no parent then we should kill ourselves
		if parent == nil then destroy_entity(ent); return end

		-- max hp = {0.0, 0.5, 0.0}
		-- 0   hp = {1.0, 0.0, 0.0}
		local child = entity_get_userdata(ent)
		local hp_pct = entity_get_hp(parent) / entity_get_max_hp(parent)

		local parent_scalex, parent_scaley = entity_get_scale(parent)
		local parent_scale_factor = ((parent_scalex + parent_scaley) * 0.5) / GLOBAL_UNIFORM_SCALE

		local full_hp_scaley = prefabs.healthbar.impl_scale[2] / (1.0 + prefabs.healthbar.impl_bar_margin)
		local diff = prefabs.healthbar.impl_scale[2] - full_hp_scaley
		local scalex = prefabs.healthbar.impl_scale[1] - diff

		entity_set_position(ent, 0, 0.8 * parent_scale_factor * GLOBAL_UNIFORM_SCALE)
		entity_set_scale(ent, prefabs.healthbar.impl_scale[1] * parent_scale_factor, prefabs.healthbar.impl_scale[2] * parent_scale_factor)

		entity_set_colour_tint(child, 1.0 - hp_pct, 0.5 * hp_pct, 0.0)
		entity_set_scale(child, hp_pct * scalex * parent_scale_factor, full_hp_scaley * parent_scale_factor)

		entity_set_position(child, -(1.0 - hp_pct) * scalex * GLOBAL_UNIFORM_SCALE * parent_scale_factor, 0.0)
	end,
	base_health = 1,
	movement_speed = 0,
	power = 1
}