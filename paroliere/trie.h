#ifndef TRIE_H_INCLUDED
#define TRIE_H_INCLUDED

#include <stdbool.h>

typedef struct s{
    char letter;
    bool endOfWord;
    struct s* sons[26];
} node;

node tree;

void insert_elem ( char *str );

void delete_tree ( node *root );

int search_elem ( char *str );

#endif // TRIE_H_INCLUDED
