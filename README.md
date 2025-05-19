# Welcome to Ledmington's personal repository
This repository is an archive for my personal little projects. Too little to get each one a dedicated repo. These programs are mostly implementation of strange data structures or algorithms to beat some real-world games. I usually implement these in a day or two, just for fun. For this reason, they may have some serious bugs. If you happen to find one, please let me know.

I hope you'll find something useful :)

## Games
- [36_cards](https://github.com/Ledmington/personal/tree/master/36_cards) is a little Python script that calculates the probability of winning a card solitaire game called "36 cards".
- [escaping_the_black_hole](https://github.com/Ledmington/personal/tree/master/escaping_the_black_hole) is a C++ algorithm which simulates random matches for a board game I invented (which is called "Escaping the Black Hole") in order to find the best deck composition.
- [merge](https://github.com/Ledmington/personal/tree/master/merge) is a minimal "merge" game like [Little Alchemy](https://littlealchemy.com).
- [orologio](https://github.com/Ledmington/personal/tree/master/orologio) is a little Python script that calculates the probability of winning a card solitaire game called "Orologio" (the clock).
- [paroliere](https://github.com/Ledmington/personal/tree/master/paroliere) is a C implementation of a brute-force algorithm that finds all possible italian words in a game of "Paroliere" (it's very similar to Ruzzle).
- [secret_code](https://github.com/Ledmington/personal/tree/master/secret_code) is a C implementation of an algorithm that wins the game "Secret Code" against you (the game is similar to Mastermind).
- [word_guesser](https://github.com/Ledmington/personal/tree/master/word_guesser) is a simple C game that guesses your word through binary search.

## Algorithms and Data Structures
- [buckets](https://github.com/Ledmington/personal/tree/master/buckets) is a comparison between different algorithms solving a problem which I think is NP-hard but I cannot find in the literature
- [codegen](https://github.com/Ledmington/personal/tree/master/codegen) is a personal java implementation of a simulated annealing algorithm trying to find the optimal code which solves a given problem
- [compression](https://github.com/Ledmington/personal/tree/master/compression) is an attempt to invent a simple compression algorithm based on repeated subsequences of bits: it contains both the original Java implementation and a new C99 implementation
- [conjugate_gradient](https://github.com/Ledmington/personal/tree/master/conjugate_gradient) is a personal Rust implementation of the [Conjugate Gradient method](https://en.wikipedia.org/wiki/Conjugate_gradient_method) and an attempt to make it more stable by using the [Kahan sum](https://en.wikipedia.org/wiki/Kahan_summation_algorithm) in the dot product
- [cuthill_mckee](https://github.com/Ledmington/personal/tree/master/cuthill_mckee) is a personal Rust implementation and visualization of the [Cuthill-McKee algorithm](https://en.wikipedia.org/wiki/Cuthill%E2%80%93McKee_algorithm)
- [diet](https://github.com/Ledmington/personal/tree/master/diet) is a personal java implementation of the simplex algorithm applied to solve the "diet problem"
- [disk_bench](https://github.com/Ledmington/personal/tree/master/disk_bench) is a comparison of different disk scheduling algorithms in Rust
- [kmeans](https://github.com/Ledmington/personal/tree/master/kmeans) is a little Rust implementation of the K-Means algorithm with plot generation.
- [kruskal_clustering](https://github.com/Ledmington/personal/tree/master/kruskal_clustering) is a Rust implementation of a simple clustering algorithm based on Kruskal's algorithm for the minimum vertex cover.
- [lev](https://github.com/Ledmington/personal/tree/master/lev) is a personal C++ implementation of various algorithms to compute the Levenshtein distance and one simple command-line application to find the most similar lines in a file
- [linear_regression](https://github.com/Ledmington/personal/tree/master/linear_regression) is a personal Rust implementation of a linear regressor with a plot of the dataset
- [mergesort](https://github.com/Ledmington/personal/tree/master/mergesort) is a performance comparison between 5 different C implementations of the Merge-Sort algorithm.
- [min](https://github.com/Ledmington/personal/tree/master/min) is an experimental non-deterministic minimization algorithm.
- [necklace](https://github.com/Ledmington/personal/tree/master/necklace) is a little Python script that compares some algorithms which solve the necklace problem.
- [pca](https://github.com/Ledmington/personal/tree/master/pca) is a personal Rust implementation of a simple algorithm to compute the principal component analysis on an hypercube, project it onto 2d space and then plot it
- [poly](https://github.com/Ledmington/personal/tree/master/poly) is a java implementation of Newton's method to find roots of a polynomial in the complex plane
- [quad_tree](https://github.com/Ledmington/personal/tree/master/quad-tree) is a performance comparison between serial and parallel implementations of the naive algorithm and a quad-tree when counting the collisions between some 2D circles.
- [queue](https://github.com/Ledmington/personal/tree/master/queue) contains my personal implementation of Circular Queue with Fixed Size (CQFS, for short) with tests and a little benchmark.
- [trie](https://github.com/Ledmington/personal/tree/master/trie) is a C implementation of the "Trie" data structure.
- [xor](https://github.com/Ledmington/personal/tree/master/xor) contains the 128, 256 and 512-bit implementations of the simple XOR hash algorithm.

## Others
- [aliquot](https://github.com/Ledmington/personal/tree/master/aliquot) is a little java program to compute the [Aliquot sequence](https://en.wikipedia.org/wiki/Aliquot_sequence).
- [bandwidth](https://github.com/Ledmington/personal/tree/master/bandwidth) is a personal C/C++ implementation of a memory bandwidth benchmark to be compared to the de-facto standard: [STREAM](https://www.cs.virginia.edu/stream/FTP/Code/stream.c).
- [chem_speller](https://github.com/Ledmington/personal/tree/master/chem_speller) is a python implementation of the algorithm to spell words only with atomic symbols which doesn't cheat unlike [the original](https://www.chemspeller.com/index.html?).
- [crawler](https://github.com/Ledmington/personal/tree/master/crawler) is a little Python script that randomly explores all web pages, starting from one link and looking for new URLs inside the HTML code received.
- [gitstats](https://github.com/Ledmington/personal/tree/master/gitstats) is a little Rust program which generates a plot with the number of files and lines of code over time in a git repository.
- [magicsquare](https://github.com/Ledmington/personal/tree/master/magicsquare) is a parallel C+OpenMP program that searches non-trivial magic squares.
- [matmul](https://github.com/Ledmington/personal/tree/master/matmul) is a little benchmark to compare the speed of a naive matrix multiplication between: C++, vectorized C++ (with google highway), Java and vectorized Java (with the Vector API)
- [mincorr](https://github.com/Ledmington/personal/tree/master/mincorr) is a little C program that finds the array with minimum correlation with a given array.
- [mulper](https://github.com/Ledmington/personal/tree/master/mulper) is a little java program to find the smallest numbers with the highest multiplicative persistence.
- [multiply_strings](https://github.com/Ledmington/personal/tree/master/multiply_strings) is a little experiment with the concept of "multiplying strings".
