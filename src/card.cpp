#include "card.hpp"
#include "prefab.hpp"

#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/os/file.hpp"
#include "tz/io/image.hpp"

#include <unordered_map>

namespace game
{
	tz::io::image_header cardbase_creature_img;
	std::string cardbase_creature_data;

	tz::io::image_header cardbase_spell_img;
	std::string cardbase_spell_data;

	std::uint32_t facedown_card_sprite;

	tz::v3f card_colours[] =
	{
		tz::v3f{0.2f, 0.45f, 0.1f},
		tz::v3f{0.6f, 0.6f, 0.2f},
		tz::v3f{0.3f, 0.4f, 0.9f},
		tz::v3f{0.8f, 0.1f, 0.95f},
		tz::v3f{0.7f, 0.1f, 0.15f},
	};

	void impl_cache_prefab_sprite(std::string_view prefab_name);
	tz::v3f impl_card_colour(unsigned int power)
	{
		return card_colours[std::clamp(power, 1u, static_cast<unsigned int>(sizeof(card_colours) / sizeof(card_colours[0]))) - 1];
	}

	struct edited_img
	{
		std::string imgdata;
		std::uint32_t texture_id;
	};
	// sprite_image_cache[prefab name] = string containing image data (but matching cardbase_creature_img - expected equal to cardbase_creature_data + scaled image data representing the appearance of the prefab i.e the sprite on the card.)
	std::unordered_map<std::string, edited_img> sprite_image_cache{};
	void card_setup()
	{
		std::string cardbase_creature_filedata = tz_must(tz::os::read_file("./res/images/cardbase_creature.png"));
		cardbase_creature_img = tz_must(tz::io::image_info(tz::view_bytes(cardbase_creature_filedata)));
		cardbase_creature_data.resize(cardbase_creature_img.data_size_bytes);
		tz::io::parse_image(tz::view_bytes(cardbase_creature_filedata), tz::view_bytes(cardbase_creature_data));

		std::string cardbase_spell_filedata = tz_must(tz::os::read_file("./res/images/cardbase_spell.png"));
		cardbase_spell_img = tz_must(tz::io::image_info(tz::view_bytes(cardbase_spell_filedata)));
		cardbase_spell_data.resize(cardbase_spell_img.data_size_bytes);
		tz::io::parse_image(tz::view_bytes(cardbase_spell_filedata), tz::view_bytes(cardbase_spell_data));

		facedown_card_sprite =  game::render::create_image_from_data(cardbase_spell_img, tz::view_bytes(cardbase_spell_data), "card_facedown");
	}

	render::handle create_card_sprite(card c, bool draggable)
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
			impl_cache_prefab_sprite(c.name);
			card_img = sprite_image_cache.at(c.name).texture_id;
		}
		game::render::quad_flag flags = game::render::quad_flag::match_image_ratio;
		if(draggable)
		{
			flags = flags | game::render::quad_flag::draggable;
		}

		return render::create_quad({.scale = tz::v2f::filled(0.2f), .texture_id0 = card_img, .colour = {1.0f, 1.0f, 1.0f}, .layer = card_layer}, flags);
	}

	render::handle create_card_sprite_facedown(card c, bool draggable)
	{
		game::render::quad_flag flags = game::render::quad_flag::match_image_ratio;
		if(draggable)
		{
			flags = flags | game::render::quad_flag::draggable;
		}

		tz::v3f card_colour = impl_card_colour(game::get_prefab(c.name).power);

		return render::create_quad({.scale = tz::v2f::filled(0.2f), .texture_id0 = facedown_card_sprite, .colour = card_colour, .layer = card_layer}, flags);
	}

	std::string card_get_name(card c)
	{
		return game::get_prefab(c.name).display_name;
	}

	std::string card_get_description(card c)
	{
		return game::get_prefab(c.name).description;
	}

	void impl_cache_prefab_sprite(std::string_view prefab_name)
	{
		std::string prefab_name_str{prefab_name};
		game::prefab prefab = game::get_prefab(prefab_name_str);

		// get the first frame of the idle flipbook.
		std::string& data = sprite_image_cache[prefab_name_str].imgdata;
		// set to cardbase data.
		tz::io::image_header header;
		if(prefab.spell_decoration)
		{
			header = cardbase_spell_img;
			data = cardbase_spell_data;
		}
		else
		{
			header = cardbase_creature_img;
			data = cardbase_creature_data;
		}
		// now get the sprite image itself.
		std::uint32_t first_texture_id = game::render::flipbook_get_frames(prefab.idle).front();

		tz::v3f card_colour = impl_card_colour(prefab.power);

		for(std::size_t i = 0; i < data.size(); i += 4)
		{
			auto r = static_cast<float>(static_cast<unsigned char>(data[i + 0])) * card_colour[0];
			auto g = static_cast<float>(static_cast<unsigned char>(data[i + 1])) * card_colour[1];
			auto b = static_cast<float>(static_cast<unsigned char>(data[i + 2])) * card_colour[2];
			data[i + 0] = static_cast<char>(r);
			data[i + 1] = static_cast<char>(g);
			data[i + 2] = static_cast<char>(b);
		}

		tz::io::image_header first_imghdr = game::render::get_image_info(first_texture_id);
		std::span<const std::byte> first_imgdata = game::render::get_image_data(first_texture_id);
		// edit 'data' to have the new image data.
		constexpr float prefab_scale_factor = 6.0f;

		// Calculate the top-left position to center the prefab on the card base
		int offset_x = (header.width - (first_imghdr.width * prefab_scale_factor)) / 2;
		constexpr int nudge_upwards_slightly = -20;
		int offset_y = (header.height + nudge_upwards_slightly - (first_imghdr.height * prefab_scale_factor)) / 2;

		// Copy prefab image pixels onto the card base
		for (unsigned int y = 0; y < prefab_scale_factor * first_imghdr.height; ++y)
		{
			for (unsigned int x = 0; x < prefab_scale_factor * first_imghdr.width; ++x)
			{
				// Calculate indices in each image
				int src_x = static_cast<int>(x / prefab_scale_factor);
				int src_y = static_cast<int>(y / prefab_scale_factor);
				int prefab_idx = (src_y * first_imghdr.width + src_x) * 4;
				int card_idx = ((offset_y + y) * header.width + (offset_x + x)) * 4;

				if(static_cast<unsigned char>(first_imgdata[prefab_idx + 3]) == 0)
				{
					// this pixel is fully transparent.
					// check neighbours. if *any* neighbours are non-transparent, then we should be an outline colour
					// otherwise, nothing
					bool all_neighbours_transparent = true;
					for(int dx = -1; all_neighbours_transparent && dx <= 1; dx++)
					{
						if(src_x + dx < 0 || src_x + dx >= first_imghdr.width)
						{
							break;
						}
						for(int dy = -1; dy <= 1; dy++)
						{
							if(src_y + dy < 0 || src_y + dy >= first_imghdr.height || (dx == 0 && dy == 0))
							{
								continue;
							}
							int neighbour_idx = ((src_y + dy) * first_imghdr.width + ((src_x + dx))) * 4;
							if(static_cast<unsigned char>(first_imgdata[neighbour_idx + 3]) != 0)
							{
								all_neighbours_transparent = false;
								break;
							}
						}
					}
					if(!all_neighbours_transparent)
					{
						for(int i = 0; i < 3; i++)
						{
							data[card_idx + i] = 57 * card_colour[i];
						}
						data[card_idx + 3] = std::numeric_limits<unsigned char>::max();
					}
					continue;
				}

				// Overwrite the card pixel data with prefab data
				for (int i = 0; i < 4; ++i)
				{
					data[card_idx + i] = static_cast<char>(first_imgdata[prefab_idx + i]);
					if(i < 3)
					{
						data[card_idx + i] = static_cast<char>(static_cast<float>(static_cast<unsigned char>(data[card_idx + i])) * prefab.colour_tint[i]);
					}
				}
			}
		}


		// finally create the new resultant image.
		sprite_image_cache[prefab_name_str].texture_id = game::render::create_image_from_data(header, tz::view_bytes(data), std::format("cardsprite_{}", prefab_name_str));
	}
}
