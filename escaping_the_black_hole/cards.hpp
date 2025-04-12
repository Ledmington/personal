#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

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
	 * discard an engine of any kind otherwise they lose.
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

constexpr std::array<card, 19> all_cards = {
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

const size_t max_card_name_length = std::transform_reduce(
	all_cards.cbegin(), all_cards.cend(), 0, [](size_t a, size_t b) { return std::max(a, b); },
	[](const card& c) {
		std::ostringstream oss;
		oss << c;
		return oss.str().length();
	});

/**
 * Returns a random card from all that can exist.
 */
card random_card(std::mt19937& rnd) {
	std::uniform_int_distribution<size_t> dist{0, all_cards.size() - 1};
	return all_cards.at(dist(rnd));
}
