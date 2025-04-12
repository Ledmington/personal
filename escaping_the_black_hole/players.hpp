#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "cards.hpp"

enum class player_state : uint8_t {
	// Player killed by the Meteor Shower.
	DEAD,
	// Player alive and playing.
	ALIVE,
	// Player managed to escape the black hole, is not affected by the game anymore.
	SAVED
};

struct player {
	player_state state;
	std::vector<card> hand;
};

const std::array<std::string, 6> players_names = {"Alice", "Bob", "Charlie", "Dylan", "Emily", "Frank"};
