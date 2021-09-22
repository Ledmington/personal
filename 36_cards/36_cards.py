'''
This program calulates the difficulty of a card solitaire game called "36 Cards"
by calculating the probability to get a winning permutation of the card's deck.
This game is completely deterministic and requires no skill or choice by the
player.

This game is played on a deck of "Piacentine" italian cards. The deck is composed
of 40 cards, divided in 4 symbols each of these ones divided in 10 values.
The symbols are: denari (diamonds), bastoni (clubs), spade (spades) and coppe
(cups). The 10 values are (in ascending order): ace, two, three, four, five, six,
seven, jack, horse and king.
The values will be replaced with integer numbers: ace->1, jack->8, horse->9, king->10.
The symbols will be replaced with integer numbers: denari->1, bastoni->2, spade->3,
coppe->4.

The rules are simple:
1) Shuffle the deck
2) Place the first 36 cards, face down, in a grid with 4 rows and 9 columns.
   From now on we say that "the row corresponds to the symbol" and "the column
   corresponds to the value". This means that the place, in the grid, at the
   n-th row and the m-th column belongs to the card with value m and symbol n.
3) Place the remaining 4 cards, face down, on the table near the grid. These
   4 cards will be called "side-deck".
4) Draw one card from the side-deck.
   If there are no more cards in the side-deck and there are still some face-down
   cards in the grid, you lost.
   Otherwise, you won.
5) If it is a king, discard it and go to step 4. Otherwise, place it face-up on
   the spot that "belongs to it" and take the face-down card that was there.
   Repeat this step until you find a king.
'''

import random

num_iterations = 1234567
games_won = 0
deck = []

# Deck setup
for value in range(1,11):
	for symbol in range(1,5):
		deck.append((value,symbol))

for _ in range(0,num_iterations):
	# Shuffling the deck
	random.shuffle(deck)
	
	# Placing the 36 cards in the grid
	grid = deck[:36]
	side_deck = deck[36:]
	flipped = [False]*36

	# For each card in the side-deck
	for i in range(0,4):
		card = side_deck[i]
		# Repeat until it's a king
		while card[0] != 10:
			idx = (card[1]-1)*9 + card[0] -1
			flipped[idx] = not flipped[idx]
			card = grid[idx]

	if all(flipped):
		games_won += 1

print(f"Games won/total: {games_won}/{num_iterations}")
print(f"Probability to win: {games_won/num_iterations}")