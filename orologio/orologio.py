'''
This program calulates the difficulty of a card solitaire game called "Orologio"
(the clock) by calculating the probability to get a winning permutation of the
card's deck. This game is completely deterministic and requires no skill or choice
by the player.

This game is played on a deck of "Piacentine" italian card. The deck is composed
of 40 cards, divided in 4 symbols each of these ones divided in 10 values.
The symbols are: denari (diamonds), bastoni (clubs), spade (spades) and coppe (cups).
The 10 values are (in ascending order): ace, two, three, four, five, six, seven,
jack, horse and king.
The symbols are not useful in this game so we are not considering them.
The values will be replaced with integer numbers: ace->1, jack->8, horse->9, king->10.

The rules are simple.
1) Shuffle the deck.
2) Place the first 12 cards face up in a circle, like they were the hours on a
   clock.
   If there are four cards of the same value, you can't win so go back to rule 1.
3) Now keep the other 28 cards face down in your hand.
4) Place the first on, face down, inside the circle and then place the second one,
   face up, next to it.
5) Flip all the cards in the circle with the same value of the face-up card you
   just drew from the deck.
6) Repeat steps 4 and 5 until either you run out of cards or there are no face-up
   cards in the circle.
   If there are no face-up cards in the circle, you won.
   Otherwise, you lost.
'''

import random
from collections import Counter

num_iterations = 1234567
games_total = 0
games_won = 0
games_unwinnable = 0
deck = []

# Deck setup
for i in range(1,11):
	deck.append(i)
	deck.append(i)
	deck.append(i)
	deck.append(i)

for _ in range(0,num_iterations):
	# Shuffling the deck
	random.shuffle(deck)

	# Placing the first 12 cards
	on_table_cards = deck[0:12]
	hand_deck = deck[12:]

	# Checking if there are 4 cards of equal value
	if 4 in Counter(on_table_cards).values():
		games_unwinnable += 1
		continue

	games_total += 1

	# Eliminating the even-numbered cards
	hand_deck = hand_deck[1::2]

	# Checking if all cards on the table can be flipped
	if set(set(on_table_cards) & set(hand_deck)) == set(on_table_cards):
		games_won += 1

print(f"Games won/total: {games_won}/{games_total}")
print(f"Probability to win: {games_won/games_total}")
print(f"Games unwinnable/total: {games_unwinnable}/{num_iterations}")
print(f"Probability to get an unwinnable permutation: {games_unwinnable/num_iterations}")