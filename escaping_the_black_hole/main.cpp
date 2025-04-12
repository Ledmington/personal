#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[31m"

#include "cards.hpp"
#include "game.hpp"
#include "players.hpp"
#include "utils.hpp"

enum class match_result : uint8_t {
	NORMAL,
	FIRST_TURN_VICTORY,
	END_BY_METEOR_SHOWER,
	WIN_BY_METEOR_SHOWER,
	MAX_TURNS_REACHED
};

std::ostream& operator<<(std::ostream& os, const match_result& mr) {
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

struct statistics {
	size_t first_turn_victories;
	size_t games_ended_by_meteor_shower;
	size_t games_won_by_meteor_shower;
	size_t max_turns_reached;
};

std::vector<card> create_deck(const std::unordered_map<card, size_t>& num_cards) {
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

void print_deck_verbose(const std::unordered_map<card, size_t>& deck) {
	std::ios_base::fmtflags f(std::cout.flags());
	const std::string sep = " +-------------------------+----+";
	std::cout << sep << std::endl;
	size_t total{0};
	for (auto const& [c, num] : deck) {
		total += num;
		std::cout << " | " << std::setw(max_card_name_length) << std::left << c << " | " << std::setw(2) << std::right
				  << num << " |" << std::endl;
	}
	std::cout << sep << std::endl;
	std::cout << " | Total                   | " << std::setw(2) << total << " |" << std::endl;
	std::cout << sep << std::endl;
	std::cout.flags(f);
}

void print_deck_short(const std::unordered_map<card, size_t>& deck) {
	std::cout << "[";
	size_t total{0};
	for (auto const& [c, num] : deck) {
		total += num;
		std::cout << c << ":" << num << ";";
	}
	std::cout << "] Total: " << total << std::endl;
}

match_result simulate_match(const std::vector<card>& original_deck, const size_t n_players,
							const size_t first_turn_cards, const size_t seed) {
	assert(original_deck.size() > 0);
	assert(n_players >= 3);

	// This represents the number of "rounds"
	constexpr size_t max_turns = 20;

	std::vector<card> deck(original_deck.size());
	std::copy(original_deck.begin(), original_deck.end(), deck.begin());
	std::mt19937 rng{seed};
	std::shuffle(deck.begin(), deck.end(), rng);

	game_state game{deck, std::vector<card>(), std::vector<player>(n_players), false};

	// Draw first hand
	for (size_t p{0}; p < n_players; p++) {
		game.players.at(p).state = player_state::ALIVE;
		for (size_t i{0}; i < first_turn_cards; i++) {
			draw_card_from_deck(game, p, rng);
		}
	}

	size_t turn{1};
	for (; turn <= max_turns; turn++) {
		if (check_defeat_by_meteor_shower(game)) {
			return match_result::END_BY_METEOR_SHOWER;
		}
		if (check_victory_by_meteor_shower(game)) {
			return match_result::WIN_BY_METEOR_SHOWER;
		}

		if (turn == 1) {
			bool all_win = true;
			for (size_t p{0}; p < n_players; p++) {
				if (!check_victory(game, p)) {
					all_win = false;
				}
			}
			if (all_win) {
				return match_result::FIRST_TURN_VICTORY;
			}
		}

		// If zero alive-and-playing players, we stop
		if (count_alive_players(game) == 0) {
			break;
		}

		for (size_t p{0}; p < n_players; p++) {
			if (game.players.at(p).state != player_state::ALIVE) {
				continue;
			}

			if (game.electromagnetic_pulse.has_value() && game.electromagnetic_pulse.value() == p) {
				game.electromagnetic_pulse = std::nullopt;
			}
			if (check_victory(game, p)) {
				game.players.at(p).state = player_state::SAVED;
				for (const card& crd : game.players.at(p).hand) {
					game.discard_pile.push_back(crd);
				}
				game.players.at(p).hand.clear();
				continue;
			}
			draw_card_from_deck(game, p, rng);

			// Each move is represented by an index in the player's hand, or -1
			// to "do nothing".
			int move = 0;
			do {
				if (contains(game.players.at(p).hand, card::METEOR_SHOWER)) {
					play_card(game, p, card::METEOR_SHOWER, rng);

					if (count_alive_players(game) == 0) {
						return match_result::END_BY_METEOR_SHOWER;
					}
					if (count_alive_players(game) == 1) {
						return match_result::WIN_BY_METEOR_SHOWER;
					}

					// Check if this player killed himself
					if (game.players.at(p).state != player_state::ALIVE) {
						break;
					}
				}

				// Collecting available moves
				std::vector<int> moves;
				moves.push_back(-1);
				for (size_t i{0}; i < game.players.at(p).hand.size(); i++) {
					if (is_card_playable(game, p, game.players.at(p).hand.at(i))) {
						moves.push_back(static_cast<int>(i));
					}
				}
				assert(moves.size() >= 1);
				assert(contains(moves, -1));
				assert(std::unordered_set<int>(moves.begin(), moves.end()).size() == moves.size());

				std::uniform_int_distribution<size_t> dist{0, moves.size() - 1};
				move = moves.at(dist(rng));
				assert(move == -1 || static_cast<size_t>(move) < game.players.at(p).hand.size());
				if (move == -1) {
				} else {
					const card card_to_play = game.players.at(p).hand.at(static_cast<size_t>(move));
					play_card(game, p, card_to_play, rng);
				}
			} while (move != -1);
		}
	}

	if (turn >= max_turns) {
		return match_result::MAX_TURNS_REACHED;
	}

	return match_result::NORMAL;
}

statistics simulate_with(const size_t n_players, const size_t matches_to_simulate, const size_t first_turn_cards,
						 const std::vector<card>& original_deck) {
	assert(n_players >= 3);

	// std::cout << std::endl;
	// std::cout << "Simulating with " << n_players << " players." << std::endl;

	size_t first_turn_victories = 0;
	size_t games_ended_by_meteor_shower = 0;
	size_t games_won_by_meteor_shower = 0;
	size_t max_turns_reached = 0;

#pragma omp parallel for schedule(auto) default(none)                       \
	shared(n_players, first_turn_cards, matches_to_simulate, original_deck) \
	reduction(+ : first_turn_victories, games_ended_by_meteor_shower, games_won_by_meteor_shower, max_turns_reached)
	for (size_t i = 0; i < matches_to_simulate; i++) {
		const match_result result = simulate_match(original_deck, n_players, first_turn_cards, i);
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

	/*
	std::cout << " First Turn Victories:         " << std::setw(7) << first_turn_victories << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed << std::setprecision(2)
			  << ((static_cast<double>(first_turn_victories) / static_cast<double>(matches_to_simulate)) * 100.0)
			  << "%)." << std::endl;
	std::cout << " Games ended by Meteor Shower: " << std::setw(7) << games_ended_by_meteor_shower << " / "
			  << std::setw(7) << matches_to_simulate << " (" << std::fixed << std::setprecision(2)
			  << ((static_cast<double>(games_ended_by_meteor_shower) / static_cast<double>(matches_to_simulate)) *
				  100.0)
			  << "%)." << std::endl;
	std::cout << " Games won by Meteor Shower:   " << std::setw(7) << games_won_by_meteor_shower << " / "
			  << std::setw(7) << matches_to_simulate << " (" << std::fixed << std::setprecision(2)
			  << ((static_cast<double>(games_won_by_meteor_shower) / static_cast<double>(matches_to_simulate)) * 100.0)
			  << "%)." << std::endl;
	std::cout << " Games too long:               " << std::setw(7) << max_turns_reached << " / " << std::setw(7)
			  << matches_to_simulate << " (" << std::fixed << std::setprecision(2)
			  << ((static_cast<double>(max_turns_reached) / static_cast<double>(matches_to_simulate)) * 100.0) << "%)."
			  << std::endl;
			  */

	return {first_turn_victories, games_ended_by_meteor_shower, games_won_by_meteor_shower, max_turns_reached};
}

double evaluate(const std::unordered_map<card, size_t>& num_cards, const size_t num_players) {
	std::vector<card> deck = create_deck(num_cards);

	// TODO: change this to be optimized as well
	const size_t first_turn_cards = 5;
	const size_t matches_to_simulate = 100'000;

	// std::cout << std::endl;
	// std::cout << "Cards to draw on first turn: " << first_turn_cards << "." << std::endl;
	// std::cout << std::endl;
	// print_deck_short(num_cards);

	const statistics stats = simulate_with(num_players, matches_to_simulate, first_turn_cards, deck);

	return static_cast<double>(stats.first_turn_victories + stats.games_ended_by_meteor_shower +
							   stats.games_won_by_meteor_shower + stats.max_turns_reached);
}

void search(const std::unordered_map<card, size_t>& starting_point,
			std::unordered_map<card, std::pair<size_t, size_t>>& limits, const size_t starting_players) {
	assert(starting_players >= 3 && starting_players <= 6);
#ifndef NDEBUG
	for (const auto& [c, num] : starting_point) {
		assert(num >= limits[c].first && num <= limits[c].second);
	}
#endif

	std::cout << "Starting configuration:" << std::endl;
	print_deck_verbose(starting_point);
	std::cout << std::endl;

	std::unordered_map<card, size_t> x = starting_point;
	double fx = evaluate(x, starting_players);
	constexpr size_t max_attempts = 100;

	for (size_t attempt{0}; attempt < max_attempts; attempt++) {
		std::unordered_map<card, size_t> y = x;
		card card_to_change;
		int step = 0;
		double best_fy = std::numeric_limits<double>::infinity();
		for (const auto& [c, num] : y) {
			if (num < limits[c].second) {
				y[c]++;
				// TODO: change also the number of players
				const double fy_forward = evaluate(y, starting_players);
				y[c]--;

				if (fy_forward < best_fy) {
					best_fy = fy_forward;
					card_to_change = c;
					step = 1;
				}
			}

			if (num > limits[c].first) {
				y[c]--;
				// TODO: change also the number of players
				const double fy_back = evaluate(y, starting_players);
				y[c]++;

				if (fy_back < best_fy) {
					best_fy = fy_back;
					card_to_change = c;
					step = -1;
				}
			}
		}

		if (best_fy < fx) {
			x[card_to_change] += step;
			fx = best_fy;

			std::ios_base::fmtflags f(std::cout.flags());
			std::cout << "Step " << std::setw(3) << attempt << "/" << max_attempts << ": " << (step > 0 ? '+' : '-')
					  << std::abs(step) << " " << std::setw(max_card_name_length) << std::left << card_to_change
					  << " (score: " << std::setw(6) << fx << ")" << std::endl;
			std::cout.flags(f);
		} else {
			break;
		}
	}

	std::cout << std::endl;
	std::cout << "Final best:" << std::endl;
	print_deck_verbose(x);
	std::cout << std::endl;
}

int main(int argc, const char**) {
	if (argc > 1) {
		std::cerr << std::endl;
		std::cerr << "WARNING: you have passed arguments to the command-line but they are not needed." << std::endl;
		std::cerr << "Ignoring command-line arguments." << std::endl;
		std::cerr << std::endl;
	}

	// Starting point
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

	// Limits
	std::unordered_map<card, std::pair<size_t, size_t>> limits{
		{card::SUPPLIES, {1, 10}},
		{card::MISSILES, {1, 10}},
		{card::METEOR_SHOWER, {1, 10}},
		{card::BARTER, {1, 10}},
		{card::FUEL_CELL, {1, 20}},
		{card::COMPUTER, {1, 10}},
		{card::QUANTUM_COMPUTER, {1, 10}},
		{card::SWAP, {1, 10}},
		{card::ELECTROMAGNETIC_PULSE, {1, 10}},
		{card::THREAT, {1, 10}},
		{card::COMBUSTION_ENGINE, {1, 10}},
		{card::ELECTRIC_ENGINE, {1, 10}},
		{card::SOLAR_PANELS, {1, 20}},
		{card::LASER_GUN, {1, 10}},
		{card::FREE_REPAIR, {1, 10}},
		{card::SCRAP, {1, 10}},
		{card::EXCHANGE_OF_INFORMATION, {1, 10}},
		{card::ENERGY_SHIELD, {1, 10}},
		{card::ESPIONAGE, {1, 10}},
	};

	search(num_cards, limits, 4);

	return 0;
}