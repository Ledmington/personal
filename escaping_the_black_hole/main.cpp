#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Set to 1 for debug
#define LOGGING 0

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[31m"

enum class card : uint8_t {
	/**
	 * All players draw a card in order starting from whoever played this card.
	 */
	SUPPLIES,

	/**
	 * Shoot to two other players of your choice. If they don't protect
	 * themselves, they discard a card.
	 */
	MISSILES,

	/**
	 * Play this card as soon as you draw it from the deck. Everybody needs to
	 * discard an engine of any kind of they lose.
	 */
	METEOR_SHOWER,

	/**
	 * Pick a player with at least one card in hand. You two exchange a card
	 * from your hands.
	 */
	BARTER,

	/**
	 * It is needed to make engines work.
	 */
	FUEL_CELL,

	/**
	 * Discard this card with an Engine and 2 Fuel Cells to Escape the Black
	 * Hole.
	 */
	COMPUTER,

	/**
	 * You can use this instead of a normal computer but you need 1 extra Fuel
	 * Cell.
	 */
	QUANTUM_COMPUTER,

	/**
	 * Pick a player and exchange your hand with his/hers.
	 */
	SWAP,

	/**
	 * Place this card in front of you and remove it at the beginning of your
	 * next turn (before drawing from the deck). When this card is on the table,
	 * Solar Panels and Electric Engines do not work and cannot be used.
	 */
	ELECTROMAGNETIC_PULSE,

	/**
	 * Pick a player with at least one card in hand. Say the name of a card: if
	 * he/she has it, he/she must give it to you.
	 */
	THREAT,

	/**
	 * Discard this card, a Computer and 2 Fuel Cells to Escape the Black Hole.
	 */
	COMBUSTION_ENGINE,

	/**
	 * Discard 3 Electric Engines and a Computer to Escape the Black Hole.
	 */
	ELECTRIC_ENGINE,

	/**
	 * You can discard 2 Solar Panels instead of a Fuel Cell when you need to.
	 */
	SOLAR_PANELS,

	/**
	 * Shoot a player of your choice, if he/she doesn't protect himself/herself,
	 * he/she discards a card of his/her choice.
	 */
	LASER_GUN,

	/**
	 * Discard this card and a Scrap to draw a card.
	 */
	FREE_REPAIR,

	/**
	 * This card has no effect.
	 */
	SCRAP,

	/**
	 * Every player takes a card from his/her hand and places it face-down on
	 * the table. Shuffle all these cards and give them back to the players one
	 * by one starting from whoever played this card. The players which have
	 * zero cards do not participate in this.
	 */
	EXCHANGE_OF_INFORMATION,

	/**
	 * Use this card when somebody is shooting at you to protect yourself.
	 */
	ENERGY_SHIELD,

	/**
	 * Take a card from the hand of a player of your choice.
	 */
	ESPIONAGE
};

std::ostream& operator<<(std::ostream& os, const card c) {
	switch (c) {
		case card::SUPPLIES:
			os << "Supplies";
			break;
		case card::MISSILES:
			os << "Missiles";
			break;
		case card::METEOR_SHOWER:
			os << "Meteor Shower";
			break;
		case card::BARTER:
			os << "Barter";
			break;
		case card::COMBUSTION_ENGINE:
			os << "Combustion Engine";
			break;
		case card::ELECTRIC_ENGINE:
			os << "Electric Engine";
			break;
		case card::FUEL_CELL:
			os << "Fuel Cell";
			break;
		case card::SWAP:
			os << "Swap";
			break;
		case card::SOLAR_PANELS:
			os << "Solar Panels";
			break;
		case card::LASER_GUN:
			os << "Laser Gun";
			break;
		case card::QUANTUM_COMPUTER:
			os << "Quantum Computer";
			break;
		case card::THREAT:
			os << "Threat";
			break;
		case card::COMPUTER:
			os << "Computer";
			break;
		case card::FREE_REPAIR:
			os << "Free Repair";
			break;
		case card::SCRAP:
			os << "Scrap";
			break;
		case card::EXCHANGE_OF_INFORMATION:
			os << "Exchange of Information";
			break;
		case card::ENERGY_SHIELD:
			os << "Energy Shield";
			break;
		case card::ESPIONAGE:
			os << "Espionage";
			break;
		case card::ELECTROMAGNETIC_PULSE:
			os << "Electromagnetic Pulse";
			break;
	}
	return os;
}

enum class match_result : uint8_t {
	NORMAL,
	FIRST_TURN_VICTORY,
	END_BY_METEOR_SHOWER,
	WIN_BY_METEOR_SHOWER,
	MAX_TURNS_REACHED
};

std::ostream& operator<<(std::ostream& os, const match_result mr) {
	switch (mr) {
		case match_result::NORMAL:
			os << "NORMAL";
			break;
		case match_result::FIRST_TURN_VICTORY:
			os << "FIRST_TURN_VICTORY";
			break;
		case match_result::END_BY_METEOR_SHOWER:
			os << "END_BY_METEOR_SHOWER";
			break;
		case match_result::WIN_BY_METEOR_SHOWER:
			os << "WIN_BY_METEOR_SHOWER";
			break;
		default:
			std::abort();
			break;
	}
	return os;
}

struct player {
	bool alive;
	std::vector<card> hand;
};

struct game_state {
	std::vector<card> deck;
	std::vector<card> discard_pile;
	std::vector<player> players;
	std::optional<size_t> electromagnetic_pulse;
};

const std::unordered_map<card, size_t> num_cards{
	{card::SUPPLIES, 3},
	{card::MISSILES, 2},
	{card::METEOR_SHOWER, 1},
	{card::BARTER, 4},
	{card::FUEL_CELL, 10},
	{card::COMPUTER, 2},
	{card::QUANTUM_COMPUTER, 2},
	{card::SWAP, 2},
	{card::ELECTROMAGNETIC_PULSE, 1},
	{card::THREAT, 2},
	{card::COMBUSTION_ENGINE, 4},
	{card::ELECTRIC_ENGINE, 9},
	{card::SOLAR_PANELS, 10},
	{card::LASER_GUN, 6},
	{card::FREE_REPAIR, 4},
	{card::SCRAP, 5},
	{card::EXCHANGE_OF_INFORMATION, 2},
	{card::ENERGY_SHIELD, 6},
	{card::ESPIONAGE, 6},
};

card random_card(std::mt19937& rnd) {
	std::vector<card> cards{
		card::SUPPLIES,
		card::MISSILES,
		card::METEOR_SHOWER,
		card::BARTER,
		card::FUEL_CELL,
		card::COMPUTER,
		card::QUANTUM_COMPUTER,
		card::SWAP,
		card::ELECTROMAGNETIC_PULSE,
		card::THREAT,
		card::COMBUSTION_ENGINE,
		card::ELECTRIC_ENGINE,
		card::SOLAR_PANELS,
		card::LASER_GUN,
		card::FREE_REPAIR,
		card::SCRAP,
		card::EXCHANGE_OF_INFORMATION,
		card::ENERGY_SHIELD,
		card::ESPIONAGE,
	};
	std::uniform_int_distribution<size_t> dist{0, cards.size() - 1};
	return cards[dist(rnd)];
}

std::vector<card> create_deck() {
	std::vector<card> deck;
	size_t count = 0;
	for (auto const& [c, num] : num_cards) {
		assert(num >= 1);
		for (size_t i{0}; i < num; i++) {
			deck.push_back(c);
		}
		count += num;
	}
	assert(deck.size() == count);
	return deck;
}

std::string get_player_name(const size_t player_index) {
	switch (player_index) {
		case 0:
			return "Alice";
		case 1:
			return "Bob";
		case 2:
			return "Charlie";
		case 3:
			return "Dylan";
		case 4:
			return "Emily";
		case 5:
			return "Frank";
		default:
			return "UNKNOWN";
	}
}

template <typename T>
std::string get_string(const std::vector<T>& v) {
	std::ostringstream oss;
	oss << '[';
	if (v.size() > 0) {
		oss << v[0];
		for (size_t i{1}; i < v.size(); i++) {
			oss << ", " << v[i];
		}
	}
	oss << ']';
	return oss.str();
}

template <typename T>
bool contains(const std::vector<T>& v, const T x) {
	return std::find(v.begin(), v.end(), x) != v.end();
}

template <typename T>
void remove_from(std::vector<T>& v, const T c) {
	const auto it = std::find(v.begin(), v.end(), c);
#ifndef NDEBUG
	const size_t count = std::count(v.begin(), v.end(), c);
#endif
	assert(count > 0);
	v.erase(it);
	assert(static_cast<size_t>(std::count(v.begin(), v.end(), c)) == count - 1);
}

template <typename T>
void remove_from(std::vector<T>& v, const size_t index) {
	assert(index < v.size());
	v.erase(v.begin() + index);
}

template <typename T>
T choose_random(std::mt19937& rnd, const std::vector<T>& v) {
	assert(v.size() > 0);

	if (v.size() == 1) {
		return v.at(0);
	}

	std::uniform_int_distribution<size_t> dist{0, v.size() - 1};
	return v.at(dist(rnd));
}

bool check_victory_by_meteor_shower(const game_state& state) {
	assert(state.players.size() > 0);
#if defined(LOGGING) && LOGGING == 1
	std::cout << "Checking for victory by meteor shower ... " << std::flush;
#endif	// LOGGING

	bool has_meteor_shower = false;
	for (size_t p{0}; p < state.players.size(); p++) {
		if (state.players[p].alive &&
			contains(state.players[p].hand, card::METEOR_SHOWER)) {
			has_meteor_shower = true;
			break;
		}
	}

	if (!has_meteor_shower) {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "none." << std::endl;
#endif	// LOGGING
		return false;
	}

	size_t n_total_engines = 0;
	for (size_t p{0}; p < state.players.size(); p++) {
		if (!state.players[p].alive) {
			continue;
		}
		n_total_engines +=
			std::count_if(state.players[p].hand.begin(),
						  state.players[p].hand.begin(), [](const card& c) {
							  return c == card::COMBUSTION_ENGINE ||
									 c == card::ELECTRIC_ENGINE;
						  });
	}

	// A game is considered "won" by a meteor shower if all players get
	// eliminated by it except one.
	if (n_total_engines == 1) {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "yes." << std::endl;
#endif	// LOGGING
		return true;
	} else {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "none." << std::endl;
#endif	// LOGGING
		return false;
	}
}

bool check_defeat_by_meteor_shower(const game_state& state) {
#if defined(LOGGING) && LOGGING == 1
	std::cout << "Checking for defeat by meteor shower ... " << std::flush;
#endif	// LOGGING

	bool has_meteor_shower = false;
	for (size_t p{0}; p < state.players.size(); p++) {
		if (state.players[p].alive &&
			contains(state.players[p].hand, card::METEOR_SHOWER)) {
			has_meteor_shower = true;
			break;
		}
	}

	if (!has_meteor_shower) {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "none." << std::endl;
#endif	// LOGGING
		return false;
	}

	size_t n_total_engines = 0;
	for (size_t p{0}; p < state.players.size(); p++) {
		if (!state.players[p].alive) {
			continue;
		}
		n_total_engines +=
			std::count_if(state.players[p].hand.begin(),
						  state.players[p].hand.begin(), [](const card& c) {
							  return c == card::COMBUSTION_ENGINE ||
									 c == card::ELECTRIC_ENGINE;
						  });
	}

	// A game is considered to be ended by an asteroid if there
	// are not enough engines for the players.
	// This is an optimal situation so this probability is to be
	// considered an upper-bound.
	if (n_total_engines == 0) {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "yes." << std::endl;
#endif	// LOGGING
		return true;
	} else {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "none." << std::endl;
#endif	// LOGGING
		return false;
	}
}

bool check_victory(const game_state& state, const size_t current_player_index) {
	assert(state.players[current_player_index].alive);
#if defined(LOGGING) && LOGGING == 1
	std::cout << "  Checking for victory ... " << std::flush;
#endif	// LOGGING

	const std::vector<card>& hand = state.players[current_player_index].hand;

	for (size_t p{0}; p < hand.size(); p++) {
		const size_t n_computers =
			std::count_if(hand.begin(), hand.begin(),
						  [](const card& c) { return c == card::COMPUTER; });
		const size_t n_quantum_computers = std::count_if(
			hand.begin(), hand.begin(),
			[](const card& c) { return c == card::QUANTUM_COMPUTER; });
		const size_t n_fuel_cells =
			std::count_if(hand.begin(), hand.begin(),
						  [](const card& c) { return c == card::FUEL_CELL; });
		const size_t n_solar_panels = std::count_if(
			hand.begin(), hand.begin(),
			[](const card& c) { return c == card::SOLAR_PANELS; });
		const size_t n_combustion_engines = std::count_if(
			hand.begin(), hand.begin(),
			[](const card& c) { return c == card::COMBUSTION_ENGINE; });
		const size_t n_electric_engines = std::count_if(
			hand.begin(), hand.begin(),
			[](const card& c) { return c == card::ELECTRIC_ENGINE; });

		// Temporary to ease victory checking
		const size_t energy = 2 * n_fuel_cells + n_solar_panels;

		// Checking First Turn Victories
		// WARNING: this check works only if each player draws at least 4
		// cards since the winning combination with the least amount of
		// cards is Combustion Engine, Computer and 2 Fuel Cells.
		if ((n_computers >= 1 && n_combustion_engines >= 1 && energy >= 4) ||
			(n_computers >= 1 && n_electric_engines >= 3) ||
			(n_quantum_computers >= 1 && n_combustion_engines >= 1 &&
			 energy >= 6) ||
			(n_quantum_computers >= 1 && n_electric_engines >= 3 &&
			 energy >= 2)) {
#if defined(LOGGING) && LOGGING == 1
			std::cout << "player " << p << " (" << get_player_name(p)
					  << ") wins." << std::endl;
#endif	// LOGGING
			return true;
		}
	}
#if defined(LOGGING) && LOGGING == 1
	std::cout << "none." << std::endl;
#endif	// LOGGING

	return false;
}

void draw_card_from_deck(game_state& state, const size_t current_player_index,
						 std::mt19937& rnd) {
	assert(current_player_index < state.players.size());
	assert(state.players[current_player_index].alive);

	// The player to pick up the last card in the deck, shuffles the discard
	// pile and puts it as the deck
	if (state.deck.size() == 0) {
		assert(state.discard_pile.size() > 0);

#if defined(LOGGING) && LOGGING == 1
		std::cout << "  Deck has no more cards, "
				  << get_player_name(current_player_index)
				  << " shuffles the discard pile." << std::endl;
#endif	// LOGGING

		std::shuffle(state.discard_pile.begin(), state.discard_pile.end(), rnd);
		for (size_t i{0}; i < state.discard_pile.size(); i++) {
			state.deck.push_back(state.discard_pile.at(i));
		}
		state.discard_pile.clear();
	}

	assert(state.deck.size() > 0);

	const card c = state.deck.at(0);
	state.deck.erase(state.deck.begin());
	state.players[current_player_index].hand.push_back(c);
}

size_t count_alive_players(const game_state& state) {
	assert(state.players.size() > 0);
	size_t count = 0;
	for (size_t i{0}; i < state.players.size(); i++) {
		if (state.players[i].alive) {
			count++;
		}
	}
	return count;
}

bool is_card_playable(const game_state& state,
					  const size_t current_player_index, const card& c) {
	assert(current_player_index < state.players.size());
	assert(state.players[current_player_index].alive);
	assert(contains(state.players[current_player_index].hand, c));

	switch (c) {
		// Always playable
		case card::SUPPLIES:
		case card::METEOR_SHOWER:
			return true;

			// These cards are not playable by themselves, they are either
			// "forced to be played" in response to other events (like Energy
			// Shield) or are used to win, in which case they are checked for
			// use before this function is called.
		case card::ENERGY_SHIELD:
		case card::COMBUSTION_ENGINE:
		case card::ELECTRIC_ENGINE:
		case card::COMPUTER:
		case card::QUANTUM_COMPUTER:
		case card::SOLAR_PANELS:
		case card::FUEL_CELL:
		case card::SCRAP:
			return false;

			// Can be played if this player has at least one other card and
			// there is at least one other player with at least one card.
		case card::EXCHANGE_OF_INFORMATION:
			assert(state.players[current_player_index].alive);
			if (state.players[current_player_index].hand.size() < 2) {
				return false;
			}
			for (size_t p{0}; p < state.players.size(); p++) {
				if (state.players[p].alive && p != current_player_index &&
					state.players[p].hand.size() >= 1) {
					return true;
				}
			}
			return false;

			// These cards need at least one player (not the current one) with
			// at least one card in hand
		case card::THREAT:
		case card::ESPIONAGE:
		case card::LASER_GUN:
		case card::SWAP:
			for (size_t p{0}; p < state.players.size(); p++) {
				if (state.players[p].alive && p != current_player_index &&
					state.players[p].hand.size() >= 1) {
					return true;
				}
			}
			return false;

		case card::MISSILES: {
			// If there are only 2 players, we use it as a Laser Gun
			if (count_alive_players(state) == 2) {
				for (size_t p{0}; p < state.players.size(); p++) {
					if (state.players[p].alive && p != current_player_index &&
						state.players[p].hand.size() >= 1) {
						return true;
					}
				}
				return false;
			}

			size_t count = 0;
			for (size_t p{0}; p < state.players.size(); p++) {
				if (state.players[p].alive && p != current_player_index &&
					state.players[p].hand.size() >= 1) {
					count++;
				}
			}
			return count >= 2;
		}

			// The current player needs to have one card (other than BARTER) and
			// another player needs to have 1 card
		case card::BARTER:
			if (state.players[current_player_index].hand.size() < 2) {
				return false;
			}
			for (size_t p{0}; p < state.players.size(); p++) {
				if (state.players[p].alive && p != current_player_index &&
					state.players[p].hand.size() >= 1) {
					return true;
				}
			}
			return false;

		case card::FREE_REPAIR:
			return contains(state.players[current_player_index].hand,
							card::SCRAP);

		case card::ELECTROMAGNETIC_PULSE:
			return !state.electromagnetic_pulse.has_value();

		default:
			std::cerr << std::endl
					  << "ERROR: unknown card (" << c << ")." << std::endl;
			std::abort();
			return false;
	}
}

std::vector<size_t> get_players_with_at_least_one_card(
	const game_state& state, const size_t current_player_index) {
	assert(current_player_index < state.players.size());
	std::vector<size_t> possible_players;
	for (size_t p{0}; p < state.players.size(); p++) {
		if (state.players[p].alive && p != current_player_index &&
			state.players[p].hand.size() >= 1) {
			possible_players.push_back(p);
		}
	}
	assert(possible_players.size() >= 1);
	return possible_players;
}

void play_card(game_state& state, const size_t current_player_index,
			   const card& c, std::mt19937& rnd) {
	assert(state.players[current_player_index].alive);
	assert(contains(state.players[current_player_index].hand, c));

	// Move the card into the discard pile
	remove_from(state.players[current_player_index].hand, c);
	state.discard_pile.push_back(c);

	switch (c) {
		case card::SUPPLIES: {
			for (size_t p{0}; p < state.players.size(); p++) {
				const size_t idx =
					(current_player_index + p) % state.players.size();
				if (!state.players[idx].alive) {
					continue;
				}

				draw_card_from_deck(state, idx, rnd);
			}
		} break;

		case card::THREAT: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);

			std::uniform_int_distribution<size_t> dist{
				0, possible_players.size() - 1};
			const size_t other_player{possible_players[dist(rnd)]};
			const card card_picked = random_card(rnd);
			const auto it =
				std::find(state.players[other_player].hand.begin(),
						  state.players[other_player].hand.end(), card_picked);
			if (it != state.players[other_player].hand.end()) {
				state.players[other_player].hand.erase(it);
				state.players[current_player_index].hand.push_back(card_picked);
			}
		} break;

		case card::EXCHANGE_OF_INFORMATION: {
			std::vector<size_t> players_involved;
			for (size_t p{0}; p < state.players.size(); p++) {
				if (state.players[p].alive &&
					state.players[p].hand.size() >= 1) {
					players_involved.push_back(p);
				}
			}
			assert(players_involved.size() >= 2 &&
				   players_involved.size() <= state.players.size());
			assert(contains(players_involved, current_player_index));

			std::vector<card> cards_picked;
			for (size_t p{0}; p < players_involved.size(); p++) {
				const card card_picked =
					choose_random(rnd, state.players[players_involved[p]].hand);
				cards_picked.push_back(card_picked);
				remove_from(state.players[players_involved[p]].hand,
							card_picked);
			}
			assert(cards_picked.size() == players_involved.size());
			std::shuffle(cards_picked.begin(), cards_picked.end(), rnd);

			for (size_t p{0}; p < players_involved.size(); p++) {
				state.players[players_involved[p]].hand.push_back(
					cards_picked[p]);
			}
		} break;

		case card::ESPIONAGE: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);

			const size_t other_player = choose_random(rnd, possible_players);
			const card card_picked =
				choose_random(rnd, state.players[other_player].hand);
			remove_from(state.players[other_player].hand, card_picked);
			state.players[current_player_index].hand.push_back(card_picked);
		} break;

		case card::BARTER: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);
			assert(possible_players.size() >= 1);
			assert(state.players[current_player_index].hand.size() >= 1);

			// Choose the other player
			const size_t other_player = choose_random(rnd, possible_players);

			// Choose the card to receive from the other player
			const card other_player_card =
				choose_random(rnd, state.players[other_player].hand);
			remove_from(state.players[other_player].hand, other_player_card);

			// Choose the card to give to the other player
			const card this_player_card =
				choose_random(rnd, state.players[current_player_index].hand);
			remove_from(state.players[current_player_index].hand,
						this_player_card);

			state.players[current_player_index].hand.push_back(
				other_player_card);
			state.players[other_player].hand.push_back(this_player_card);
		} break;

		case card::LASER_GUN: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);

			const size_t other_player = choose_random(rnd, possible_players);

			if (contains(state.players[other_player].hand,
						 card::ENERGY_SHIELD)) {
				remove_from(state.players[other_player].hand,
							card::ENERGY_SHIELD);
				state.discard_pile.push_back(card::ENERGY_SHIELD);
			} else {
				std::uniform_int_distribution<size_t> dist2{
					0, state.players[other_player].hand.size() - 1};
				const card other_player_card =
					state.players[other_player].hand[dist2(rnd)];
				remove_from(state.players[other_player].hand,
							other_player_card);
				state.discard_pile.push_back(other_player_card);
			}
		} break;

		case card::MISSILES: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);
			assert(possible_players.size() > 0);

			std::uniform_int_distribution<size_t> dist{
				0, possible_players.size() - 1};
			const size_t p1{possible_players[dist(rnd)]};

			if (contains(state.players[p1].hand, card::ENERGY_SHIELD)) {
				remove_from(state.players[p1].hand, card::ENERGY_SHIELD);
				state.discard_pile.push_back(card::ENERGY_SHIELD);
			} else {
				std::uniform_int_distribution<size_t> dist2{
					0, state.players[p1].hand.size() - 1};
				const card other_player_card =
					state.players[p1].hand[dist2(rnd)];
				remove_from(state.players[p1].hand, other_player_card);
				state.discard_pile.push_back(other_player_card);
			}

			if (possible_players.size() >= 2) {
				size_t p2;
				do {
					p2 = possible_players[dist(rnd)];
				} while (p1 == p2);
				if (contains(state.players[p2].hand, card::ENERGY_SHIELD)) {
					remove_from(state.players[p2].hand, card::ENERGY_SHIELD);
					state.discard_pile.push_back(card::ENERGY_SHIELD);
				} else {
					std::uniform_int_distribution<size_t> dist2{
						0, state.players[p2].hand.size() - 1};
					const card other_player_card =
						state.players[p2].hand[dist2(rnd)];
					remove_from(state.players[p2].hand, other_player_card);
					state.discard_pile.push_back(other_player_card);
				}
			}
		} break;

		case card::METEOR_SHOWER:
			for (size_t p{0}; p < state.players.size(); p++) {
				if (!state.players[p].alive) {
					continue;
				}

				bool saved = false;
				if (contains(state.players[p].hand, card::COMBUSTION_ENGINE)) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << get_player_name(p) << " discards a "
							  << card::COMBUSTION_ENGINE << "." << std::endl;
#endif	// LOGGING
					remove_from(state.players[p].hand, card::COMBUSTION_ENGINE);
					state.discard_pile.push_back(card::COMBUSTION_ENGINE);
					saved = true;
				} else if (contains(state.players[p].hand,
									card::ELECTRIC_ENGINE)) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << get_player_name(p) << " discards a "
							  << card::ELECTRIC_ENGINE << "." << std::endl;
#endif	// LOGGING
					remove_from(state.players[p].hand, card::ELECTRIC_ENGINE);
					state.discard_pile.push_back(card::ELECTRIC_ENGINE);
					saved = true;
				}
				if (!saved) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << ANSI_RED << get_player_name(p)
							  << " eliminated" << ANSI_RESET << "."
							  << std::endl;
#endif	// LOGGING
					state.players[p].alive = false;
					for (const card& crd : state.players[p].hand) {
						state.discard_pile.push_back(crd);
					}
					state.players[p].hand.clear();

					// If the player who played electromagnetic pulse dies, the
					// EMP is removed
					if (state.electromagnetic_pulse.has_value() &&
						state.electromagnetic_pulse.value() == p) {
						state.electromagnetic_pulse = std::nullopt;
					}
				}
			}
			break;

		case card::FREE_REPAIR:
			assert(contains(state.players[current_player_index].hand,
							card::SCRAP));
			remove_from(state.players[current_player_index].hand, card::SCRAP);
			state.discard_pile.push_back(card::SCRAP);
			draw_card_from_deck(state, current_player_index, rnd);
			break;

		case card::SWAP: {
			const std::vector<size_t> possible_players =
				get_players_with_at_least_one_card(state, current_player_index);

			std::uniform_int_distribution<size_t> dist{
				0, possible_players.size() - 1};
			const size_t other_player{possible_players[dist(rnd)]};

			std::vector<card> tmp;
			for (size_t i{0};
				 i < state.players[current_player_index].hand.size(); i++) {
				tmp.push_back(state.players[current_player_index].hand[i]);
			}
			state.players[current_player_index].hand.clear();
			for (size_t i{0}; i < state.players[other_player].hand.size();
				 i++) {
				state.players[current_player_index].hand.push_back(
					state.players[other_player].hand[i]);
			}
			state.players[other_player].hand.clear();
			for (size_t i{0}; i < tmp.size(); i++) {
				state.players[other_player].hand.push_back(tmp[i]);
			}
		} break;

		case card::ELECTROMAGNETIC_PULSE:
			assert(!state.electromagnetic_pulse.has_value());
			state.electromagnetic_pulse =
				std::optional<size_t>{current_player_index};
			break;

		default:
			std::cerr << std::endl
					  << "ERROR: unknown card (" << c << ")." << std::endl;
			std::abort();
			break;
	}
}

match_result simulate_match(const std::vector<card>& original_deck,
							const size_t n_players,
							const size_t first_turn_cards, const size_t seed) {
	assert(original_deck.size() > 0);
	assert(n_players >= 3);

	constexpr size_t max_turns = 100;

	std::vector<card> deck(original_deck.size());
	std::copy(original_deck.begin(), original_deck.end(), deck.begin());
	std::mt19937 rng{seed};
	std::shuffle(deck.begin(), deck.end(), rng);

	game_state state{deck, std::vector<card>(), std::vector<player>(n_players),
					 false};

	// Draw first hand
	for (size_t p{0}; p < n_players; p++) {
		state.players[p].alive = true;
		for (size_t i{0}; i < first_turn_cards; i++) {
			draw_card_from_deck(state, p, rng);
		}
	}

	size_t turn{1};
	for (; turn < max_turns; turn++) {
#if defined(LOGGING) && LOGGING == 1
		std::cout << "ROUND " << turn << std::endl;
		std::cout << state.deck.size() << " cards in the deck." << std::endl;
		std::cout << state.discard_pile.size() << " cards in the discard pile."
				  << std::endl;
		std::cout << "Total cards: "
				  << (state.deck.size() + state.discard_pile.size() +
					  std::accumulate(state.players.begin(),
									  state.players.end(), 0,
									  [](size_t sum, const player& p) {
										  return sum + p.hand.size();
									  }))
				  << std::endl;
		std::cout << count_alive_players(state) << " alive players."
				  << std::endl;
#endif	// LOGGING

		if (check_defeat_by_meteor_shower(state)) {
			return match_result::END_BY_METEOR_SHOWER;
		}
		if (check_victory_by_meteor_shower(state)) {
			return match_result::WIN_BY_METEOR_SHOWER;
		}

		for (size_t p{0}; p < n_players; p++) {
			if (!state.players[p].alive) {
				continue;
			}

#if defined(LOGGING) && LOGGING == 1
			std::cout << " " << get_player_name(p) << "'s turn" << std::endl;
#endif	// LOGGING

			if (state.electromagnetic_pulse.has_value() &&
				state.electromagnetic_pulse.value() == p) {
#if defined(LOGGING) && LOGGING == 1
				std::cout << " Electromagnetic Pulse removed." << std::endl;
#endif	// LOGGING
				state.electromagnetic_pulse = std::nullopt;
			}
			if (check_victory(state, p)) {
				return match_result::FIRST_TURN_VICTORY;
			}
			draw_card_from_deck(state, p, rng);

			// Each move is represented by an index in the player's hand, or -1
			// to "do nothing".
			int move = 0;
			do {
#if defined(LOGGING) && LOGGING == 1
				std::cout << "  Hand is: " << get_string(state.players[p].hand)
						  << std::endl;
#endif	// LOGGING

				if (contains(state.players[p].hand, card::METEOR_SHOWER)) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  Plays " << ANSI_BOLD << card::METEOR_SHOWER
							  << ANSI_RESET << "." << std::endl;
#endif	// LOGGING
					play_card(state, p, card::METEOR_SHOWER, rng);

					if (count_alive_players(state) == 0) {
						return match_result::END_BY_METEOR_SHOWER;
					}
					if (count_alive_players(state) == 1) {
						return match_result::WIN_BY_METEOR_SHOWER;
					}

					// Check if this player killed himself
					if (!state.players[p].alive) {
						break;
					}
				}

				// Collecting available moves
				std::vector<int> moves;
				moves.push_back(-1);
				for (size_t i{0}; i < state.players[p].hand.size(); i++) {
					if (is_card_playable(state, p, state.players[p].hand[i])) {
						moves.push_back(static_cast<int>(i));
					}
				}
				assert(moves.size() >= 1);
				assert(contains(moves, -1));
				assert(std::unordered_set<int>(moves.begin(), moves.end())
						   .size() == moves.size());

#if defined(LOGGING) && LOGGING == 1
				std::cout << "  Available moves: " << get_string(moves)
						  << std::endl;
#endif	// LOGGING

				std::uniform_int_distribution<size_t> dist{0, moves.size() - 1};
				move = moves[dist(rng)];
				assert(move == -1 || static_cast<size_t>(move) <
										 state.players[p].hand.size());
				if (move == -1) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  Does nothing." << std::endl;
#endif	// LOGGING
				} else {
					const card card_to_play =
						state.players[p].hand[static_cast<size_t>(move)];
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  Plays " << ANSI_BOLD << card_to_play
							  << ANSI_RESET << "." << std::endl;
#endif	// LOGGING
					play_card(state, p, card_to_play, rng);
				}
			} while (move != -1);
		}

#if defined(LOGGING) && LOGGING == 1
		std::cout << std::endl;
#endif	// LOGGING
	}

	if (turn >= max_turns) {
		return match_result::MAX_TURNS_REACHED;
	}

	return match_result::NORMAL;
}

void simulate_with(const size_t n_players, const size_t matches_to_simulate,
				   const size_t first_turn_cards,
				   const std::vector<card>& original_deck) {
	assert(n_players >= 3);

	std::cout << std::endl;
	std::cout << "Simulating with " << n_players << " players." << std::endl;

	size_t first_turn_victories = 0;
	size_t games_ended_by_meteor_shower = 0;
	size_t games_won_by_meteor_shower = 0;
	size_t max_turns_reached = 0;

#pragma omp parallel for schedule(auto) default(none)                       \
	shared(n_players, first_turn_cards, matches_to_simulate, original_deck) \
	reduction(+ : first_turn_victories, games_ended_by_meteor_shower,       \
				  games_won_by_meteor_shower, max_turns_reached)
	for (size_t i = 0; i < matches_to_simulate; i++) {
		const match_result result =
			simulate_match(original_deck, n_players, first_turn_cards, i);
		switch (result) {
			case match_result::FIRST_TURN_VICTORY:
				first_turn_victories++;
				break;
			case match_result::END_BY_METEOR_SHOWER:
				games_ended_by_meteor_shower++;
				break;
			case match_result::WIN_BY_METEOR_SHOWER:
				games_won_by_meteor_shower++;
				break;
			case match_result::MAX_TURNS_REACHED:
				max_turns_reached++;
				break;
			default:
				// nothing
				break;
		}
	}

	std::cout << " First Turn Victories:    " << std::setw(7)
			  << first_turn_victories << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed
			  << std::setprecision(2)
			  << ((static_cast<double>(first_turn_victories) /
				   static_cast<double>(matches_to_simulate)) *
				  100.0)
			  << "%)." << std::endl;
	std::cout << " Games ended by Asteroid: " << std::setw(7)
			  << games_ended_by_meteor_shower << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed
			  << std::setprecision(2)
			  << ((static_cast<double>(games_ended_by_meteor_shower) /
				   static_cast<double>(matches_to_simulate)) *
				  100.0)
			  << "%)." << std::endl;
	std::cout << " Games won by Asteroid:   " << std::setw(7)
			  << games_won_by_meteor_shower << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed
			  << std::setprecision(2)
			  << ((static_cast<double>(games_won_by_meteor_shower) /
				   static_cast<double>(matches_to_simulate)) *
				  100.0)
			  << "%)." << std::endl;
	std::cout << " Games too long:          " << std::setw(7)
			  << max_turns_reached << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed
			  << std::setprecision(2)
			  << ((static_cast<double>(max_turns_reached) /
				   static_cast<double>(matches_to_simulate)) *
				  100.0)
			  << "%)." << std::endl;
}

int main() {
	std::random_device dev;
	std::mt19937{dev()};

	std::vector<card> deck = create_deck();

	const size_t first_turn_cards = 5;
	const size_t matches_to_simulate = 1'000'000;

	std::cout << std::endl;
	std::cout << "Cards to draw on first turn: " << first_turn_cards << "."
			  << std::endl;
	std::cout << std::endl;
	std::cout << "--- Deck statistics ---" << std::endl;
	std::cout << " Total cards: " << deck.size() << "." << std::endl;
	for (auto const& [c, num] : num_cards) {
		std::cout << " # " << c << ": " << num << std::endl;
	}
	std::cout << std::endl;

	simulate_with(3, matches_to_simulate, first_turn_cards, deck);
	simulate_with(4, matches_to_simulate, first_turn_cards, deck);
	simulate_with(5, matches_to_simulate, first_turn_cards, deck);
	simulate_with(6, matches_to_simulate, first_turn_cards, deck);

	return 0;
}