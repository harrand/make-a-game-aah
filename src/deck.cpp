#include "deck.hpp"
#include "entity.hpp"
#include "config.hpp"
#include <optional>

namespace game
{
	constexpr float deck_card_spacing = 0.2f;
	struct deck_data
	{
		std::vector<card> cards = {};
		std::vector<render::handle> card_quads = {};
		std::vector<render::text_handle> card_tooltips = {};
		std::vector<std::optional<tz::v2f>> card_target_locations = {};
		std::optional<deck_render_info> render;
	};
	std::vector<deck_data> decks = {};
	std::vector<deck_handle> free_list = {};

	deck_handle create_deck(deck_info info)
	{
		deck_handle ret;
		if(free_list.size())
		{
			ret = free_list.back();
			free_list.pop_back();
		}
		else
		{
			ret = static_cast<tz::hanval>(decks.size());
			decks.push_back({});
		}

		auto& deck = decks[ret.peek()];
		deck.render = info.sprite.value();
		deck.card_quads.clear();
		deck.cards.clear();
		deck.card_tooltips.clear();
		deck.card_target_locations.clear();

		for(card c : info.initial_cards)
		{
			deck_add_card(ret, c);
		}

		return ret;
	}

	void destroy_deck(deck_handle deck)
	{
		free_list.push_back(deck);
		const auto& d = decks[deck.peek()];
		for(render::handle h : d.card_quads)
		{
			if(h != tz::nullhand)
			{
				game::render::destroy_quad(h);
			}
		}
		for(render::text_handle h : d.card_tooltips)
		{
			if(h != tz::nullhand)
			{
				game::render::destroy_text(h);
			}
		}
	}

	void deck_update(float delta_seconds)
	{
		for(std::size_t i = 0; i < decks.size(); i++)
		{
			auto& deck = decks[i];
			if(!deck.render.has_value())
			{
				continue;
			}
			for(std::size_t j = 0; j < deck.cards.size(); j++)
			{
				auto pos = deck.card_target_locations[j];
				if(!pos.has_value())
				{
					continue;
				}
				auto old = render::quad_get_position(deck.card_quads[j]);
				if((pos.value() - old).length() < 0.1f)
				{
					old = pos.value();
					deck.card_target_locations[j] = std::nullopt;
				}
				else
				{
					old[0] = std::lerp(old[0], pos.value()[0], delta_seconds * config_computer_play_card_drag_speed);
					old[1] = std::lerp(old[1], pos.value()[1], delta_seconds * config_computer_play_card_drag_speed);
				}
				render::quad_set_position(deck.card_quads[j], old);
			}
		}
	}

	void deck_add_card(deck_handle deckh, card c)
	{
		auto& deck = decks[deckh.peek()];	
		deck.cards.push_back(c);
		render::handle& quad = deck.card_quads.emplace_back(tz::nullhand);
		deck.card_tooltips.push_back(tz::nullhand);
		deck.card_target_locations.push_back(std::nullopt);

		if(deck.render.has_value())
		{
			std::size_t deck_size = deck.cards.size();
			float deck_spacing = deck.render.value().scale[0] * deck_card_spacing;

			auto fn = &game::create_card_sprite;
			if(deck.render.value().cards_face_down)
			{
				fn = &game::create_card_sprite_facedown;
			}

			quad = fn(c, deck.render.value_or(deck_render_info{}).player_can_play_cards);
			tz::v2f card_pos = deck.render->position;
			card_pos[0] += (deck_size * deck_spacing);
			render::quad_set_position(quad, card_pos);
			auto sc = render::quad_get_scale(quad);
			sc[0] *= deck.render->scale[0];
			sc[1] *= deck.render->scale[1];
			render::quad_set_scale(quad, sc);
		}
	}

	void deck_destroy_card(deck_handle deck, std::size_t id)
	{
		auto& d = decks[deck.peek()];
		if(d.render.has_value())
		{
			game::render::destroy_quad(d.card_quads[id]);
			if(d.card_tooltips[id] != tz::nullhand)
			{
				game::render::destroy_text(d.card_tooltips[id]);
			}
			// move next cards backwards a bit.
			for(std::size_t i = id+1; i < d.cards.size(); i++)
			{
				float deck_spacing = d.render.value().scale[0] * deck_card_spacing;
				auto pos = game::render::quad_get_position(d.card_quads[i]);
				pos[0] -= deck_spacing;
				d.card_target_locations[i] = pos;
			}
		}
		d.card_quads.erase(d.card_quads.begin() + id);
		d.card_tooltips.erase(d.card_tooltips.begin() + id);
		d.card_target_locations.erase(d.card_target_locations.begin() + id);
		d.cards.erase(d.cards.begin() + id);
	}

	void deck_swap_cards(deck_handle deckh, std::size_t id1, std::size_t id2)
	{
		// firstly remove both tooltips if they're up.
		deck_card_hide_tooltip(deckh, id1);
		deck_card_hide_tooltip(deckh, id2);
		auto& deck = decks[deckh.peek()];
		if(deck.render.has_value())
		{
			auto pos1 = render::quad_get_position(deck.card_quads[id1]);
			auto pos2 = render::quad_get_position(deck.card_quads[id2]);
			render::quad_set_position(deck.card_quads[id1], pos2);
			render::quad_set_position(deck.card_quads[id2], pos1);
		}
		std::swap(deck.cards[id1], deck.cards[id2]);
		std::swap(deck.card_quads[id1], deck.card_quads[id2]);
		std::swap(deck.card_tooltips[id1], deck.card_tooltips[id2]);
	}

	void deck_reset_card_position(deck_handle deck, std::size_t id)
	{
		const auto& d = decks[deck.peek()];
		if(!d.render.has_value())
		{
			return;
		}
		float deck_spacing = d.render.value().scale[0] * deck_card_spacing;
		auto pos = d.render.value().position;
		pos[0] += (deck_spacing * (id + 1));
		render::quad_set_position(d.card_quads[id], pos);
		render::quad_set_layer(d.card_quads[id], card_layer);
	}

	std::size_t deck_size(deck_handle deck)
	{
		return decks[deck.peek()].cards.size();
	}

	card deck_get_card(deck_handle deck, std::size_t id)
	{
		return decks[deck.peek()].cards[id];
	}

	bool deck_card_is_held(deck_handle deck, std::size_t id)
	{
		const auto& d = decks[deck.peek()];
		if(!d.render.has_value())
		{
			return false;
		}
		return game::render::quad_is_held(d.card_quads[id]);
	}

	bool deck_card_is_mouseover(deck_handle deck, std::size_t id)
	{
		const auto& d = decks[deck.peek()];
		if(!d.render.has_value())
		{
			return false;
		}
		return game::render::quad_is_mouseover(d.card_quads[id]);
	}

	render::handle deck_detach_card(deck_handle deck, std::size_t id)
	{
		render::handle ret = tz::nullhand;
		// basically destroy the card without deleting the quad.
		auto& d = decks[deck.peek()];
		if(d.render.has_value())
		{
			ret = d.card_quads[id];
			if(d.card_tooltips[id] != tz::nullhand)
			{
				game::render::destroy_text(d.card_tooltips[id]);
			}
			// move next cards backwards a bit.
			for(std::size_t i = id+1; i < d.cards.size(); i++)
			{
				float deck_spacing = d.render.value().scale[0] * deck_card_spacing;
				auto pos = game::render::quad_get_position(d.card_quads[i]);
				pos[0] -= deck_spacing;
				d.card_target_locations[i] = pos;
			}
		}
		d.card_quads.erase(d.card_quads.begin() + id);
		d.card_tooltips.erase(d.card_tooltips.begin() + id);
		d.cards.erase(d.cards.begin() + id);
		d.card_target_locations.erase(d.card_target_locations.begin() + id);

		return ret;
	}

	void deck_card_display_tooltip(deck_handle deck, std::size_t id)
	{
		auto& d = decks[deck.peek()];
		if(!d.render.has_value())
		{
			return;
		}

		auto quad = d.card_quads[id];
		tz::v2f tooltip_position = game::render::quad_get_position(quad);
		float yoffset = 0.3f;
		if(tooltip_position[1] > (1.0f - yoffset))
		{
			tooltip_position[1] -= yoffset;
		}
		else
		{
			tooltip_position[1] += yoffset;
		}

		if(d.card_tooltips[id] == tz::nullhand)
		{
			card c = deck_get_card(deck, id);
			// create text.
			std::string txt = std::format("{}\n\n{}", game::card_get_name(c), game::card_get_description(c));
			d.card_tooltips[id] = game::render::create_text("kongtext", txt, tooltip_position, tz::v2f::filled(0.03f), tz::v3f::filled(1.0f));
		}
	}

	void deck_card_hide_tooltip(deck_handle deck, std::size_t id)
	{
		auto& d = decks[deck.peek()];
		if(!d.render.has_value())
		{
			return;
		}

		if(d.card_tooltips[id] != tz::nullhand)
		{
			game::render::destroy_text(d.card_tooltips[id]);
			d.card_tooltips[id] = tz::nullhand;
		}
	}

	void deck_play_card(deck_handle deck, std::size_t id, bool player_aligned)
	{
		const auto& d = decks[deck.peek()];
		tz::v2f pos;
		if(d.render.has_value())
		{
			pos = game::render::quad_get_position(d.card_quads[id]);
		}
		else
		{
			// use hardcoded "enemy" position, as deck has no render component im assuming its an enemy card.
			pos = {1.2f, 0.0f};
		}

		card c = deck_get_card(deck, id);
		game::create_entity({.prefab_name = c.name, .player_aligned = player_aligned, .position = pos, .scale = {player_aligned ? 1.0f : -1.0f, 1.0f}});
		deck_destroy_card(deck, id);
	}
}