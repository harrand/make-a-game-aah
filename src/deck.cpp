#include "deck.hpp"
#include "entity.hpp"

namespace game
{
	constexpr float deck_card_spacing = 0.2f;
	struct deck_data
	{
		std::vector<card> cards = {};
		std::vector<render::handle> card_quads = {};
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
	}

	void deck_add_card(deck_handle deckh, card c)
	{
		auto& deck = decks[deckh.peek()];	
		deck.cards.push_back(c);
		render::handle& quad = deck.card_quads.emplace_back(tz::nullhand);

		if(deck.render.has_value())
		{
			std::size_t deck_size = deck.cards.size();
			float deck_spacing = deck.render.value().scale[0] * deck_card_spacing;

			quad = game::create_card_sprite(c, deck.render.value_or(deck_render_info{}).player_can_play_cards);
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
			// move next cards backwards a bit.
			for(std::size_t i = id+1; i < d.cards.size(); i++)
			{
				float deck_spacing = d.render.value().scale[0] * deck_card_spacing;
				auto pos = game::render::quad_get_position(d.card_quads[i]);
				pos[0] -= deck_spacing;
				game::render::quad_set_position(d.card_quads[i], pos);
			}
		}
		d.card_quads.erase(d.card_quads.begin() + id);
		d.cards.erase(d.cards.begin() + id);
	}

	void deck_swap_cards(deck_handle deckh, std::size_t id1, std::size_t id2)
	{
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