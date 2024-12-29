#pragma once

#include <optional>
#include <vector>

struct game_state {
	std::vector<card> deck;
	std::vector<card> discard_pile;
	std::vector<player> players;
	std::optional<size_t> electromagnetic_pulse;
};

size_t count_engines(const std::vector<card>& hand) {
	size_t count = 0;
	for (size_t i{0}; i < hand.size(); i++) {
		if (hand.at(i) == card::COMBUSTION_ENGINE || hand.at(i) == card::ELECTRIC_ENGINE) {
			count++;
		}
	}
	return count;
}

bool check_victory_by_meteor_shower(const game_state& game) {
	assert(game.players.size() > 0);
#if defined(LOGGING) && LOGGING == 1
	std::cout << "Checking for victory by meteor shower ... " << std::flush;
#endif	// LOGGING

	bool has_meteor_shower = false;
	for (size_t p{0}; p < game.players.size(); p++) {
		if (game.players.at(p).state == player_state::ALIVE && contains(game.players.at(p).hand, card::METEOR_SHOWER)) {
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
	for (size_t p{0}; p < game.players.size(); p++) {
		if (game.players.at(p).state != player_state::ALIVE) {
			continue;
		}
		n_total_engines += count_engines(game.players.at(p).hand);
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

bool check_defeat_by_meteor_shower(const game_state& game) {
#if defined(LOGGING) && LOGGING == 1
	std::cout << "Checking for defeat by meteor shower ... " << std::flush;
#endif	// LOGGING

	bool has_meteor_shower = false;
	for (size_t p{0}; p < game.players.size(); p++) {
		if (game.players.at(p).state == player_state::ALIVE && contains(game.players.at(p).hand, card::METEOR_SHOWER)) {
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
	for (size_t p{0}; p < game.players.size(); p++) {
		if (game.players.at(p).state != player_state::ALIVE) {
			continue;
		}
		n_total_engines += count_engines(game.players.at(p).hand);
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

bool check_victory(const game_state& game, const size_t current_player_index) {
	assert(game.players.at(current_player_index).state == player_state::ALIVE);
#if defined(LOGGING) && LOGGING == 1
	std::cout << "  Checking for victory ... " << std::flush;
#endif	// LOGGING

	const std::vector<card>& hand = game.players.at(current_player_index).hand;

	for (size_t p{0}; p < hand.size(); p++) {
		const size_t n_computers = count(hand, card::COMPUTER);
		const size_t n_quantum_computers = count(hand, card::QUANTUM_COMPUTER);
		const size_t n_fuel_cells = count(hand, card::FUEL_CELL);
		const size_t n_solar_panels = count(hand, card::SOLAR_PANELS);
		const size_t n_combustion_engines = count(hand, card::COMBUSTION_ENGINE);
		const size_t n_electric_engines = count(hand, card::ELECTRIC_ENGINE);

		// Temporary to ease victory checking
		const size_t energy = 2 * n_fuel_cells + n_solar_panels;

		// Checking First Turn Victories
		// WARNING: this check works only if each player draws at least 4
		// cards since the winning combination with the least amount of
		// cards is Combustion Engine, Computer and 2 Fuel Cells.
		if ((n_computers >= 1 && n_combustion_engines >= 1 && energy >= 4) ||
			(n_computers >= 1 && n_electric_engines >= 3) ||
			(n_quantum_computers >= 1 && n_combustion_engines >= 1 && energy >= 6) ||
			(n_quantum_computers >= 1 && n_electric_engines >= 3 && energy >= 2)) {
#if defined(LOGGING) && LOGGING == 1
			std::cout << "player " << p << " (" << players_names.at(p) << ") is saved." << std::endl;
#endif	// LOGGING
			return true;
		}
	}
#if defined(LOGGING) && LOGGING == 1
	std::cout << "none." << std::endl;
#endif	// LOGGING

	return false;
}

void draw_card_from_deck(game_state& game, const size_t current_player_index, std::mt19937& rnd) {
	assert(current_player_index < game.players.size());
	assert(game.players.at(current_player_index).state == player_state::ALIVE);

	// The player to pick up the last card in the deck, shuffles the discard
	// pile and puts it as the deck
	if (game.deck.size() == 0) {
		assert(game.discard_pile.size() > 0);

#if defined(LOGGING) && LOGGING == 1
		std::cout << "  Deck has no more cards, " << players_names.at(current_player_index)
				  << " shuffles the discard pile." << std::endl;
#endif	// LOGGING

		std::shuffle(game.discard_pile.begin(), game.discard_pile.end(), rnd);
		for (size_t i{0}; i < game.discard_pile.size(); i++) {
			game.deck.push_back(game.discard_pile.at(i));
		}
		game.discard_pile.clear();
	}

	assert(game.deck.size() > 0);

	const card c = game.deck.at(0);
	game.deck.erase(game.deck.begin());
	game.players.at(current_player_index).hand.push_back(c);
}

size_t count_alive_players(const game_state& game) {
	assert(game.players.size() > 0);
	size_t count = 0;
	for (size_t i{0}; i < game.players.size(); i++) {
		if (game.players.at(i).state == player_state::ALIVE) {
			count++;
		}
	}
	return count;
}

bool is_there_one_player_with_at_least_one_card(const game_state& game, const size_t current_player_index) {
	for (size_t p{0}; p < game.players.size(); p++) {
		if (p != current_player_index && game.players.at(p).state == player_state::ALIVE &&
			game.players.at(p).hand.size() >= 1) {
			return true;
		}
	}
	return false;
}

bool is_card_playable(const game_state& game, const size_t current_player_index, const card& c) {
	assert(current_player_index < game.players.size());
	assert(game.players.at(current_player_index).state == player_state::ALIVE);
	assert(contains(game.players.at(current_player_index).hand, c));

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
		case card::BARTER:
			if (game.players.at(current_player_index).hand.size() < 2) {
				return false;
			}
			return is_there_one_player_with_at_least_one_card(game, current_player_index);

			// These cards need at least one player (not the current one) with
			// at least one card in hand
		case card::THREAT:
		case card::ESPIONAGE:
		case card::LASER_GUN:
		case card::SWAP:
			return is_there_one_player_with_at_least_one_card(game, current_player_index);

		case card::MISSILES: {
			// If there are only 2 players, we use it as a Laser Gun
			if (count_alive_players(game) == 2) {
				return is_there_one_player_with_at_least_one_card(game, current_player_index);
			}

			size_t count = 0;
			for (size_t p{0}; p < game.players.size(); p++) {
				if (game.players.at(p).state == player_state::ALIVE && p != current_player_index &&
					game.players.at(p).hand.size() >= 1) {
					count++;
				}
			}
			return count >= 2;
		}

		case card::FREE_REPAIR:
			return contains(game.players.at(current_player_index).hand, card::SCRAP);

		case card::ELECTROMAGNETIC_PULSE:
			return !game.electromagnetic_pulse.has_value();

		default:
			std::cerr << std::endl << "ERROR: unknown card (" << c << ")." << std::endl;
			std::abort();
			return false;
	}
}

std::vector<size_t> get_players_with_at_least_one_card(const game_state& game, const size_t current_player_index) {
	assert(current_player_index < game.players.size());
	std::vector<size_t> possible_players;
	for (size_t p{0}; p < game.players.size(); p++) {
		if (game.players.at(p).state == player_state::ALIVE && p != current_player_index &&
			game.players.at(p).hand.size() >= 1) {
			possible_players.push_back(p);
		}
	}
	assert(possible_players.size() >= 1);
	return possible_players;
}

void play_card(game_state& game, const size_t current_player_index, const card& c, std::mt19937& rnd) {
	assert(game.players.at(current_player_index).state == player_state::ALIVE);
	assert(contains(game.players.at(current_player_index).hand, c));

	// Move the card into the discard pile
	remove_from(game.players.at(current_player_index).hand, c);
	game.discard_pile.push_back(c);

	switch (c) {
		case card::SUPPLIES: {
			for (size_t p{0}; p < game.players.size(); p++) {
				const size_t idx = (current_player_index + p) % game.players.size();
				if (game.players.at(idx).state != player_state::ALIVE) {
					continue;
				}

				draw_card_from_deck(game, idx, rnd);
			}
		} break;

		case card::THREAT: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);

			std::uniform_int_distribution<size_t> dist{0, possible_players.size() - 1};
			const size_t other_player{possible_players.at(dist(rnd))};
			const card card_picked = random_card(rnd);
			const auto it = std::find(game.players.at(other_player).hand.begin(),
									  game.players.at(other_player).hand.end(), card_picked);
			if (it != game.players.at(other_player).hand.end()) {
				game.players.at(other_player).hand.erase(it);
				game.players.at(current_player_index).hand.push_back(card_picked);
			}
		} break;

		case card::EXCHANGE_OF_INFORMATION: {
			std::vector<size_t> players_involved;
			for (size_t p{0}; p < game.players.size(); p++) {
				if (game.players.at(p).state == player_state::ALIVE && game.players.at(p).hand.size() >= 1) {
					players_involved.push_back(p);
				}
			}
			assert(players_involved.size() >= 2 && players_involved.size() <= game.players.size());
			assert(contains(players_involved, current_player_index));

			std::vector<card> cards_picked;
			for (size_t p{0}; p < players_involved.size(); p++) {
				const card card_picked = choose_random(rnd, game.players.at(players_involved.at(p)).hand);
				cards_picked.push_back(card_picked);
				remove_from(game.players.at(players_involved.at(p)).hand, card_picked);
			}
			assert(cards_picked.size() == players_involved.size());
			std::shuffle(cards_picked.begin(), cards_picked.end(), rnd);

			for (size_t p{0}; p < players_involved.size(); p++) {
				game.players.at(players_involved.at(p)).hand.push_back(cards_picked.at(p));
			}
		} break;

		case card::ESPIONAGE: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);

			const size_t other_player = choose_random(rnd, possible_players);
			const card card_picked = choose_random(rnd, game.players.at(other_player).hand);
			remove_from(game.players.at(other_player).hand, card_picked);
			game.players.at(current_player_index).hand.push_back(card_picked);
		} break;

		case card::BARTER: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);
			assert(possible_players.size() >= 1);
			assert(game.players.at(current_player_index).hand.size() >= 1);

			// Choose the other player
			const size_t other_player = choose_random(rnd, possible_players);

			// Choose the card to receive from the other player
			const card other_player_card = choose_random(rnd, game.players.at(other_player).hand);
			remove_from(game.players.at(other_player).hand, other_player_card);

			// Choose the card to give to the other player
			const card this_player_card = choose_random(rnd, game.players.at(current_player_index).hand);
			remove_from(game.players.at(current_player_index).hand, this_player_card);

			game.players.at(current_player_index).hand.push_back(other_player_card);
			game.players.at(other_player).hand.push_back(this_player_card);
		} break;

		case card::LASER_GUN: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);

			const size_t other_player = choose_random(rnd, possible_players);

			if (contains(game.players.at(other_player).hand, card::ENERGY_SHIELD)) {
				remove_from(game.players.at(other_player).hand, card::ENERGY_SHIELD);
				game.discard_pile.push_back(card::ENERGY_SHIELD);
			} else {
				std::uniform_int_distribution<size_t> dist2{0, game.players.at(other_player).hand.size() - 1};
				const card other_player_card = game.players.at(other_player).hand.at(dist2(rnd));
				remove_from(game.players.at(other_player).hand, other_player_card);
				game.discard_pile.push_back(other_player_card);
			}
		} break;

		case card::MISSILES: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);
			assert(possible_players.size() > 0);

			std::uniform_int_distribution<size_t> dist{0, possible_players.size() - 1};
			const size_t p1{possible_players.at(dist(rnd))};

			if (contains(game.players.at(p1).hand, card::ENERGY_SHIELD)) {
				remove_from(game.players.at(p1).hand, card::ENERGY_SHIELD);
				game.discard_pile.push_back(card::ENERGY_SHIELD);
			} else {
				std::uniform_int_distribution<size_t> dist2{0, game.players.at(p1).hand.size() - 1};
				const card other_player_card = game.players.at(p1).hand.at(dist2(rnd));
				remove_from(game.players.at(p1).hand, other_player_card);
				game.discard_pile.push_back(other_player_card);
			}

			if (possible_players.size() >= 2) {
				size_t p2;
				do {
					p2 = possible_players.at(dist(rnd));
				} while (p1 == p2);
				if (contains(game.players.at(p2).hand, card::ENERGY_SHIELD)) {
					remove_from(game.players.at(p2).hand, card::ENERGY_SHIELD);
					game.discard_pile.push_back(card::ENERGY_SHIELD);
				} else {
					std::uniform_int_distribution<size_t> dist2{0, game.players.at(p2).hand.size() - 1};
					const card other_player_card = game.players.at(p2).hand.at(dist2(rnd));
					remove_from(game.players.at(p2).hand, other_player_card);
					game.discard_pile.push_back(other_player_card);
				}
			}
		} break;

		case card::METEOR_SHOWER:
			for (size_t p{0}; p < game.players.size(); p++) {
				if (game.players.at(p).state != player_state::ALIVE) {
					continue;
				}

				bool saved = false;
				if (contains(game.players.at(p).hand, card::COMBUSTION_ENGINE)) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << players_names.at(p) << " discards a " << card::COMBUSTION_ENGINE << "."
							  << std::endl;
#endif	// LOGGING
					remove_from(game.players.at(p).hand, card::COMBUSTION_ENGINE);
					game.discard_pile.push_back(card::COMBUSTION_ENGINE);
					saved = true;
				} else if (contains(game.players.at(p).hand, card::ELECTRIC_ENGINE)) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << players_names.at(p) << " discards a " << card::ELECTRIC_ENGINE << "."
							  << std::endl;
#endif	// LOGGING
					remove_from(game.players.at(p).hand, card::ELECTRIC_ENGINE);
					game.discard_pile.push_back(card::ELECTRIC_ENGINE);
					saved = true;
				}
				if (!saved) {
#if defined(LOGGING) && LOGGING == 1
					std::cout << "  " << ANSI_RED << players_names.at(p) << " eliminated" << ANSI_RESET << "."
							  << std::endl;
#endif	// LOGGING
					game.players.at(p).state = player_state::DEAD;
					for (const card& crd : game.players.at(p).hand) {
						game.discard_pile.push_back(crd);
					}
					game.players.at(p).hand.clear();

					// If the player who played electromagnetic pulse dies, the
					// EMP is removed
					if (game.electromagnetic_pulse.has_value() && game.electromagnetic_pulse.value() == p) {
						game.electromagnetic_pulse = std::nullopt;
					}
				}
			}
			break;

		case card::FREE_REPAIR:
			assert(contains(game.players.at(current_player_index).hand, card::SCRAP));
			remove_from(game.players.at(current_player_index).hand, card::SCRAP);
			game.discard_pile.push_back(card::SCRAP);
			draw_card_from_deck(game, current_player_index, rnd);
			break;

		case card::SWAP: {
			const std::vector<size_t> possible_players = get_players_with_at_least_one_card(game, current_player_index);

			std::uniform_int_distribution<size_t> dist{0, possible_players.size() - 1};
			const size_t other_player{possible_players.at(dist(rnd))};

			std::vector<card> tmp;
			for (size_t i{0}; i < game.players.at(current_player_index).hand.size(); i++) {
				tmp.push_back(game.players.at(current_player_index).hand.at(i));
			}
			game.players.at(current_player_index).hand.clear();
			for (size_t i{0}; i < game.players.at(other_player).hand.size(); i++) {
				game.players.at(current_player_index).hand.push_back(game.players.at(other_player).hand.at(i));
			}
			game.players.at(other_player).hand.clear();
			for (size_t i{0}; i < tmp.size(); i++) {
				game.players.at(other_player).hand.push_back(tmp.at(i));
			}
		} break;

		case card::ELECTROMAGNETIC_PULSE:
			assert(!game.electromagnetic_pulse.has_value());
			game.electromagnetic_pulse = std::optional<size_t>{current_player_index};
			break;

		default:
			std::cerr << std::endl << "ERROR: unknown card (" << c << ")." << std::endl;
			std::abort();
			break;
	}
}
