prefabs.warbear =
{
	display_name = "War Bear",
	description = "A ferocious, armoured brown bear.",
	base_health = 100,
	movement_speed = 0.5,
	base_cooldown = 2.0,
	base_damage = 3,
	power = 5,
	taunt = true,
	undead_variant = "undead_bear",
	idle =
	{
		fps = 1,
		loop = true,
		frames =
		{
			"creature/warbear/idle0.png",
			"creature/warbear/idle1.png"
		}
	},
	move_horizontal =
	{
		fps = 5,
		loop = true,
		frames =
		{
			"creature/warbear/idle0.png",
			"creature/warbear/side0.png",
			"creature/warbear/idle0.png",
			"creature/warbear/side1.png",
		}
	},
	attack =
	{
		fps = 2,
		loop = true,
		frames =
		{
			"creature/warbear/attack0.png",
			"creature/warbear/attack1.png",
		}
	}
}
