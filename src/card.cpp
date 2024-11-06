#include "card.hpp"
#include "creature.hpp"

#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/os/file.hpp"
#include "tz/io/image.hpp"

#include <unordered_map>

namespace game
{
	tz::io::image_header cardbase_img;
	std::string cardbase_data;

	void impl_cache_creature_sprite(std::string_view creature_name);

	struct edited_img
	{
		std::string imgdata;
		std::uint32_t texture_id;
	};
	// sprite_image_cache[creature name] = string containing image data (but matching cardbase_img - expected equal to cardbase_data + scaled image data representing the appearance of the creature i.e the sprite on the card.)
	std::unordered_map<std::string, edited_img> sprite_image_cache{};
	void card_setup()
	{
		std::string cardbase_filedata = tz_must(tz::os::read_file("./res/images/cardbase.png"));
		cardbase_img = tz_must(tz::io::image_info(tz::view_bytes(cardbase_filedata)));
		cardbase_data.resize(cardbase_img.data_size_bytes);
		tz::io::parse_image(tz::view_bytes(cardbase_filedata), tz::view_bytes(cardbase_data));
	}

	render::handle create_card_sprite(card c)
	{
		std::uint32_t card_img;
		auto cache_iter = sprite_image_cache.find(c.name);
		if(cache_iter != sprite_image_cache.end())
		{
			// already cached this img
			card_img = cache_iter->second.texture_id;
		}
		else
		{
			// havent cached this yet. create it, cache it and we wil use that texture id.
			impl_cache_creature_sprite(c.name);
			card_img = sprite_image_cache.at(c.name).texture_id;
		}

		return render::create_quad({.scale = tz::v2f::filled(0.2f), .texture_id = card_img, .colour = {1.0f, 0.05f, 0.1f}}, game::render::quad_flag::draggable | game::render::quad_flag::match_image_ratio);
	}

	void impl_cache_creature_sprite(std::string_view creature_name)
	{
		std::string creature_name_str{creature_name};
		game::creature_prefab prefab = game::get_creature_prefab(creature_name_str);

		// get the first frame of the idle flipbook.
		std::string& data = sprite_image_cache[creature_name_str].imgdata;
		// set to cardbase data.
		data = cardbase_data;
		// now get the sprite image itself.
		std::uint32_t first_texture_id = game::render::flipbook_get_frames(prefab.idle).front();

		tz::io::image_header first_imghdr = game::render::get_image_info(first_texture_id);
		std::span<const std::byte> first_imgdata = game::render::get_image_data(first_texture_id);
		// edit 'data' to have the new image data.
		// todo: superimpose the images in a meaningful way rather than whatever the fuck this is
		for(std::size_t i = 0; i < data.size(); i++)
		{
			if(i % 8 == 0)
			{
				data[i] += 128;
			}
		}

		// finally create the new resultant image.
		sprite_image_cache[creature_name_str].texture_id = game::render::create_image_from_data(cardbase_img, tz::view_bytes(data), std::format("cardsprite_{}", creature_name_str));
	}
}