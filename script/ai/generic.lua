ai = ai or {}

ai.wander_distance = 0.5

ai.rand = function(lo, hi)
    return lo + math.random()  * (hi - lo);
end

ai.wander = function(ent)
	local tarx, tary = entity_get_target_location(ent)
	if tarx == nil and tary == nil then
		-- choose a random location and set that as the target
		local posx, posy = entity_get_position(ent)
		entity_set_target_location(ent, posx + ai.rand(-ai.wander_distance * 0.5, ai.wander_distance * 0.5), posy + ai.rand(-ai.wander_distance * 0.5, ai.wander_distance * 0.5))
	end
end