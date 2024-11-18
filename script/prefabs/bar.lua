prefabs.bar =
{
	on_create = function(ent)
		local barscale = {0.5, 0.1}
		local margin = 0.4
		entity_set_position(ent, 0, 0.15)
		entity_set_scale(ent, barscale[1], barscale[2])
		local child = create_entity("empty")

		entity_set_colour_tint(child, 0.0, 0.5, 0.0)
		entity_set_parent(child, ent)
		local diff = barscale[2] - (barscale[2] / (1.0 + margin))
		entity_set_scale(child, barscale[1] - diff, barscale[2] / (1.0 + margin))
	end,
	base_health = 1,
	movement_speed = 0,
	power = 1
}