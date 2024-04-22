/*
	This program plays the board-game "Paroliere", which is a simpler version
	of the popular mobile game "Ruzzle".

	The grid is composed of 16 cubes disposed in a 4x4 grid.
	Each cube has a letter on each side.
	The grid is set up by throwing each one of this cubes to get a random
	consiguration.

	Each player needs to find the highest number of unique words or the longest
	ones in order to win.
	Each game has a timer, usually 1 minute.
	After that time, each player reads the words he found and, if another player
	has found the same word, both of them have to delete that word.
	After all repeated words have been deleted, each player counts its points
	and whoever has gained the most points wins.

	In "Paroliere" words have a minimum length of 3 letters.
	Longer words give more points as follows:
	+----------------------+--------+
	| Number of characters | Points |
	+----------------------+--------+
	|        3 or 4        |    1   |
	+----------------------+--------+
	|           5          |    2   |
	+----------------------+--------+
	|           6          |    3   |
	+----------------------+--------+
	|           7          |    5   |
	+----------------------+--------+
	|       8 or more      |   11   |
	+----------------------+--------+

	How to find a word?
	Words can be "built" one letter at a time, without visiting the same letter
	twice. You can move from a letter to one of its neighbors. Moving on
	diagonals is allowed.
	For example, look at this grid:
	+---+---+---+---+
	| T | O | G | E |
	+---+---+---+---+
	| U | R | C | T |
	+---+---+---+---+
	| U | Q | E | H |
	+---+---+---+---+
	| L | A | H | F |
	+---+---+---+---+

	Some words you can find are:
	get, her, the, core
	A good word you can find is "together".
	But words like "route" or "quote" do no exist.

	---------------------------------------------------------------------------

	This program is really simple.
	First, creates the grid with the letters.
	Then, reads a database file with all allowed words and fills a "trie" data
	structure with those words.
	Then, for each possible word on the grid, this program looks for it in the
	"trie".

	Compile with:
	gcc paroliere.c trie.c -o paroliere

	Run with:
	./paroliere [out_file]
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

typedef struct s2 {
	char letter;
	int x, y;
	bool visited;
	int num_adj;
	struct s2 **adj;
} cube;

typedef struct s3 {
	char *wordFound;
	struct s3 *next;
} parola;

void add_link(cube *pc1, cube *pc2);
void combinazioni(cube *pc);

void write(char let);
void cancel();

void add(char *str);

char input[16];

cube griglia[16];
char word[17] = {'\0'};

parola *set = NULL;
parola *tmp;

int main(int argc, char *argv[]) {
	char *filename = "input_words.txt";

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [out_file]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		strcpy(filename, argv[1]);
	}

	printf("Insert grid:\n");
	for (int i = 0; i < 4; i++) {
		if (4 != scanf("%c %c %c %c", &input[i * 4 + 0], &input[i * 4 + 1],
					   &input[i * 4 + 2], &input[i * 4 + 3])) {
			fprintf(stderr, "Error reading grid.\n");
			goto fine;
		}
		fflush(stdin);
	}

	printf("Creating grid...\n");
	for (int i = 0; i < 16; i++) {
		griglia[i].letter = input[i];
		griglia[i].letter = tolower(griglia[i].letter);
		griglia[i].visited = false;

		switch (i) {
			case 0:
			case 3:
			case 12:
			case 15:
				griglia[i].num_adj = 3;
				break;
			case 1:
			case 2:
			case 4:
			case 7:
			case 8:
			case 11:
			case 13:
			case 14:
				griglia[i].num_adj = 5;
				break;
			default:
				griglia[i].num_adj = 8;
				break;
		}

		griglia[i].adj =
			(struct s2 **)malloc(griglia[i].num_adj * sizeof(struct s2 *));
		for (int k = 0; k < griglia[i].num_adj; k++) griglia[i].adj[k] = NULL;
	}

	// Inserisco i vicini
	for (int i = 0; i < 16; i++) {
		// Vicino destra-sinistra
		if (i % 4 <= 2) add_link(&griglia[i], &griglia[i + 1]);

		// Vicino su-giu
		if (i <= 11) add_link(&griglia[i], &griglia[i + 4]);

		// Vicino basso-destra - alto-sinistra
		if (i < 11 && i % 4 <= 2) add_link(&griglia[i], &griglia[i + 5]);

		// Vicino alto-destra - basso-sinistra
		if (i % 4 >= 1 && i < 12) add_link(&griglia[i], &griglia[i + 3]);
	}

	for (int i = 0; i < 16; i++) {
		printf(" %c", griglia[i].letter);
		if (i % 4 == 3) printf("\n");
	}

	printf("\nGrid created.\n\n");

	printf("Building database...\n");
	FILE *fin = fopen(filename, "r");
	if (fin == NULL) {
		fprintf(stderr, "Error opening file \"%s\"\n", filename);
		goto fine;
	}

	char ch;
	unsigned int num_words = 1;
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

	char **words = (char **)malloc(num_words * sizeof(char *));
	if (words == NULL) {
		fprintf(stderr, "Cannot allocate memory for words array...\n");
		goto fine;
	}
	for (int i = 0; i < num_words; i++) {
		words[i] = (char *)malloc((lengths[i] + 1) * sizeof(char));
		words[i][lengths[i]] = '\0';
	}

	rewind(fin);
	unsigned int cont = 0, pos = 0;
	while ((ch = fgetc(fin)) != EOF) {
		if (ch == '\n') {
			cont++;
			pos = 0;
		} else
			words[cont][pos++] = ch;
	}
	fclose(fin);

	for (unsigned int i = 0; i < num_words; i++) {
		insert_elem(words[i]);
	}
	printf("Database built.\n");

	// Controllo combinazioni
	printf("\n\nSearching words...\n");
	for (int i = 0; i < 16; i++) {
		for (int k = 0; k < 16; k++) {
			griglia[k].visited = false;
			word[k] = '\0';
		}
		griglia[i].visited = true;
		combinazioni(&griglia[i]);
		griglia[i].visited = false;
	}
	printf("Words found:\n\n");

	tmp = set;
	cont = 0;
	while (tmp != NULL) {
		printf("%s\n", tmp->wordFound);
		cont++;
		tmp = tmp->next;
	}
	printf("\n%d words found\n", cont);

	goto fine;
fine:
	tmp = set;
	while (tmp != NULL) {
		tmp = tmp->next;
		free(set);
		set = tmp;
	}
	delete_tree(&tree);
	free(lengths);
	for (int i = 0; i < num_words; i++) {
		free(words[i]);
	}
	free(words);
	for (int i = 0; i < 16; i++) {
		free(griglia[i].adj);
	}

	return 0;
}

void add_link(cube *pc1, cube *pc2) {
	int pos1, pos2;

	for (pos1 = 0; pc1->adj[pos1] != NULL; pos1++) {
		if (pc1->adj[pos1] == pc2) break;
	}

	for (pos2 = 0; pc2->adj[pos2] != NULL; pos2++) {
		if (pc2->adj[pos2] == pc1) break;
	}

	pc1->adj[pos1] = pc2;
	pc2->adj[pos2] = pc1;
}

void combinazioni(cube *pc) {
	write(pc->letter);
	if (strlen(word) >= 3 && search_elem(word)) {
		add(word);
	}
	for (int i = 0; i < pc->num_adj; i++) {
		if (pc->adj[i]->visited == false) {
			pc->adj[i]->visited = true;
			combinazioni(pc->adj[i]);
			pc->adj[i]->visited = false;
			cancel();
		}
	}
}

void write(char let) {
	int i;
	for (i = 0; word[i] != '\0'; i++)
		;
	word[i] = let;
	if (let == 'q') word[i + 1] = 'u';
}

void cancel() {
	int i;
	for (i = strlen(word) - 1; i > 0 && word[i] == '\0'; i--)
		;
	if (word[i] == 'u' && word[i - 1] == 'q') word[i - 1] = '\0';
	word[i] = '\0';
}

void add(char *str) {
	tmp = set;
	int found = 0;
	while (tmp != NULL) {
		if (!strcmp(str, tmp->wordFound)) {
			found = 1;
			break;
		}
		tmp = tmp->next;
	}
	if (!found) {
		tmp = (parola *)malloc(sizeof(parola));
		tmp->wordFound = (char *)malloc(strlen(str) * sizeof(char));
		strcpy(tmp->wordFound, str);
		tmp->next = set;
		set = tmp;
	}
}
