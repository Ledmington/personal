#ifndef TRIE_H_INCLUDED
#define TRIE_H_INCLUDED

#include <stdbool.h>

typedef struct _trie {
	struct _trie *father;
	bool end_of_word;
	struct _trie *sons[26];
} TrieNode;

TrieNode *trie_init(void);

void trie_insert(TrieNode *root, char *str);

long unsigned int trie_size(TrieNode *root);

void trie_delete(TrieNode *root);

bool trie_search(TrieNode *root, char *str);

#endif	// TRIE_H_INCLUDED
