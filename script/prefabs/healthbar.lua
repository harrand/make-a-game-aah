prefabs.healthbar =
{
	impl_scale = {0.5, 0.1},
	impl_bar_margin = 0.6,
	has_tooltip = false,
	combat = false,
	attackable = false,
	on_create = function(ent)
		entity_set_scale(ent, prefabs.healthbar.impl_scale[1], prefabs.healthbar.impl_scale[2])
		entity_set_colour_tint(ent, 1.0, 1.0, 1.0)
		entity_set_layer(ent, 1)

		local child = create_entity("empty")
		entity_set_colour_tint(child, 0.0, 0.5, 0.0)
		entity_set_parent(child, ent)
		entity_set_userdata(ent, child)
		entity_set_layer(child, 2)
		-- set the layers so healthbars tend to always display above other sprites (as its technically UI)
	end,
	on_destroy = function(ent)
		local child = entity_get_userdata(ent)
		if entity_exists(child) then
			destroy_entity(child)
		end
	end,
	on_update = function(ent, delta_seconds)
		-- remember: userdata is the child bar
		-- parent is the thing we're monitoring
		local parent = entity_get_parent(ent)
		local child = entity_get_userdata(ent)

		-- max hp = {0.0, 0.5, 0.0}
		-- 0   hp = {1.0, 0.0, 0.0}
		local hp_pct = entity_get_hp(parent) / entity_get_max_hp(parent)

		local parent_scalex, parent_scaley = entity_get_scale(parent)
		local parent_scale_factor = ((parent_scalex + parent_scaley) * 0.5) / 1

		local full_hp_scaley = prefabs.healthbar.impl_scale[2] / (1.0 + prefabs.healthbar.impl_bar_margin)
		local diff = prefabs.healthbar.impl_scale[2] - full_hp_scaley
		local scalex = prefabs.healthbar.impl_scale[1] - diff

		entity_set_position(ent, 0, 0.8 * parent_scale_factor * 1)
		entity_set_scale(ent, prefabs.healthbar.impl_scale[1] * parent_scale_factor, prefabs.healthbar.impl_scale[2] * parent_scale_factor)

		entity_set_colour_tint(child, 1.0 - hp_pct, 0.5 * hp_pct, 0.0)
		entity_set_scale(child, hp_pct * scalex * parent_scale_factor, full_hp_scaley * parent_scale_factor)

		entity_set_position(child, -(1.0 - hp_pct) * scalex * 1 * parent_scale_factor, 0.0)
	end,
	base_health = 1,
	movement_speed = 0,
	power = 1
}
