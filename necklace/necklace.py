'''
	This program containes the implementation of two algorithms that
	solve the "stolen necklace problem". (I don't know if it's known otherwise)

	Here is the problem.
	Two thieves have stolen a necklace made from some gems of various types.
	They want to cut it in some parts and take each one some of the parts
	such that each thief receives exactly the same amount of gems for
	each type of gem.
	To add some difficulty, they want to perform at most k cuts, where k
	is the number of gem types.

	Formally, the input is:
	 - k, the number of gemtypes
	 - ngems, an array of length k containing the number of gems for each type
	 - necklace, an array of length n containing the gems on the necklace

	The output must be an array of maximum length k containing the
	indices where to cut the necklace. A cut on index i means that
	the i-th gem goes to a thief and the (i+1)-th gem goes to the other.

	Constraints:
	k is an integer >= 2
	the number of each gem is >0 and even

	
	The algorithms implemented are:
	 - bruteforce, pretty explanatory :)
	 - greedy, a linear algorithm that always finds the optimal solution.
'''

import random

def generate_input(ntypes=random.randint(2,10), maxgems=2):
	if maxgems < 2 or maxgems%2==1:
		raise Exception("maxgems must be a positive even number")
	if ntypes > 10:
		print("\nWARNING: using ntypes>10 may cause confusion when printing the string as numbers\n")

	ngems = [random.randint(1, maxgems/2)*2 for _ in range(ntypes)]
	s = ""
	for gemtype in range(ntypes):
		for _ in range(ngems[gemtype]):
			s += str(gemtype)
	l = list(s)
	random.shuffle(l)
	return {
		"ntypes":ntypes,
		"ngems":ngems,
		"s":"".join(l)
	}

def print_cuts(s, cuts):
	for i,c in enumerate(s):
		print(c, end="")
		if i in cuts:
			print("|", end="")
	print("")

def bruteforce(s, ntypes, ngems, index=0, cuts=[], output=False):
	if len(cuts) == ntypes:
		if evaluate_solution(s, cuts, output):
			return cuts
		else:
			return None

	for i in range(index, len(s)-1):
		tmp_cuts = cuts.copy()
		tmp_cuts.append(i)
		if evaluate_solution(s, tmp_cuts, output):
			return tmp_cuts
		solution = bruteforce(s, ntypes, ngems, i+1, tmp_cuts)
		if solution is not None:
			return solution

def evaluate_solution(s, cuts, output=False):
	if output:
		print(f"Evaluating: \"{s}\" + {cuts} = ", end="")
		print_cuts(s, cuts)

	thieves = ["", ""]
	thief = 0
	last_cut = 0
	for cut in cuts:
		# scorriamo fino a s[cut]
		thieves[thief] += s[last_cut:cut+1]
		thief = 1 - thief
		last_cut = cut+1

	thieves[thief] += s[last_cut:]

	# sort thieves alphabetically
	for i in range(2):
		thieves[i] = "".join(sorted(thieves[i]))

	return thieves[0] == thieves[1]

def greedy(s, ntypes, ngems):
	thieves = [[0]*ntypes, [0]*ntypes]
	thief = 0
	cuts = []

	for i in range(len(s)):
		gem = int(s[i])
		if thieves[thief][gem] +1 > ngems[gem]/2:
			thief = 1 - thief
			cuts.append(i-1)
		thieves[thief][gem] += 1

	return cuts

def main():
	for _ in range(10):
		input = generate_input(10, 4)
		print("Checking " + input["s"] + "...")
		bfcuts = bruteforce(input["s"], input["ntypes"], input["ngems"])
		print(f"bruteforce -> {bfcuts} ({len(bfcuts)} cuts)")
		print_cuts(input["s"], bfcuts)
		greedycuts = greedy(input["s"], input["ntypes"], input["ngems"])
		print(f"greedy -> {greedycuts} ({len(greedycuts)} cuts)")
		print_cuts(input["s"], greedycuts)
		print("")

if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt:
		print("\n\nBye bye")
