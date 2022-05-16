#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "trie.h"

TrieNode* trie_init ( void ) {
    TrieNode *n = (TrieNode*) malloc(sizeof(TrieNode));
    assert(n);
    for(int i=0; i<26; i++) {
        n->sons[i] = NULL;
    }
    n->father = NULL;
    n->end_of_word = false;
    return n;
}

void trie_insert (TrieNode *root, char *str) {
    TrieNode *tmp = root;
    const int len = strlen(str);
    for(int i=0; i<len; i++) {
        const int index = str[i] - 'a';
        if (tmp->sons[index] == NULL) {
            tmp->sons[index] = trie_init();
            tmp->sons[index]->father = tmp;
        }
        tmp = tmp->sons[index];
    }
    tmp->end_of_word = true;
}

void trie_delete (TrieNode *root) {
    for(int i=0; i<26; i++) {
        if( root->sons[i] != NULL ) trie_delete(root->sons[i]);
    }
    free(root);
}

bool trie_search (TrieNode *root, char* str) {
    TrieNode *tmp = root;
    const int len = strlen(str);
    int i = 0;
    while (tmp != NULL && i < len) {
        tmp = tmp->sons[str[i] - 'a'];
        i++;
    }
    return tmp != NULL && tmp->end_of_word;
}

long unsigned int trie_size (TrieNode *root) {
    if (root == NULL) return 0;
    long unsigned int sons_size = 0;
    for(int i=0; i<26; i++) {
        sons_size += trie_size(root->sons[i]);
    }
    return sizeof(root) + sons_size;
}
