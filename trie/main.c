/*
	Personal implementation of the "trie" data structure.
	This program reads a list of words from a file.
	The default one is "input_words.txt", however, a different
	file can be specified as first and only command line parameter.

	Compile with:
	gcc --std=c99 -Werror -Wall -Wpedantic *.c -o trie.exe

	Run with:
	./trie.exe [input_file]

	Example:
	./trie.exe [my_words.txt]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "trie.h"

int main(int argc, char *argv[]) {
	char *filename = "input_words.txt";
	FILE *fin;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [input_file]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		filename = argv[1];
	}

	fin = fopen(filename, "r");
	if (fin == NULL) {
		fprintf(stderr, "Fatal error: cannot open file \"%s\"\n", filename);
		goto fine;
	}

	char ch;
	unsigned int num_words = 1;
	long unsigned int words_size = 0;
	unsigned int *lengths =
		(unsigned int *)calloc(num_words, sizeof(unsigned int));
	while ((ch = fgetc(fin)) != EOF) {
		if (ch == '\n') {
			lengths = (unsigned int *)realloc(
				lengths, (++num_words) * sizeof(unsigned int));
			lengths[num_words - 1] = 0;
		} else
			lengths[num_words - 1]++;
	}
	printf("%d words.\n", num_words);

	char **words = (char **)malloc(num_words * sizeof(char *));
	if (words == NULL) {
		fprintf(stderr, "Not enough memory for words array\n");
		goto fine;
	}
	words_size += sizeof(words);
	for (int i = 0; i < num_words; i++) {
		words[i] = (char *)malloc((lengths[i] + 1) * sizeof(char));
		words[i][lengths[i]] = '\0';
		words_size += sizeof(words[i]) + ((lengths[i] + 1) * sizeof(char));
	}

	rewind(fin);
	unsigned int cont = 0, pos = 0;
	printf("Reading input file...\n\n");
	while ((ch = fgetc(fin)) != EOF) {
		if (ch == '\n') {
			cont++;
			pos = 0;
		} else
			words[cont][pos++] = ch;
	}

	printf("Building the trie...\n");
	time_t t = clock();
	TrieNode *trie_root = trie_init();
	for (unsigned int i = 0; i < num_words; i++) {
		trie_insert(trie_root, words[i]);
	}
	t = clock() - t;
	printf("Building finished.\n%.3f seconds\n\n", (double)t / CLOCKS_PER_SEC);

	printf("Total size of the array: %9lu bytes\n", words_size);
	printf("Total size of the Trie : %9lu bytes\n\n", trie_size(trie_root));

	printf("Searching all the elements...\n");
	t = clock();
	for (unsigned int i = 0; i < num_words; i++) {
		if (!trie_search(trie_root, words[i])) {
			printf("Non ho trovato \"%s\"\n", words[i]);
			getchar();
		}
	}
	t = clock() - t;
	printf("Search finished.\n%.3f seconds\n\n", (double)t / CLOCKS_PER_SEC);

fine:
	trie_delete(trie_root);
	for (int i = 0; i < cont; i++) {
		free(words[i]);
	}
	free(words);
	free(lengths);
	fclose(fin);

	return EXIT_SUCCESS;
}
