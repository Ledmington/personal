#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "trie.h"

void insert_elem (char *str) {
    node *pn = &tree;
    for(int i=0; i<strlen(str); i++) {
        if( pn->sons[ str[i]-'a' ] == NULL ) {
            pn->sons[ str[i]-'a' ] = (node*) malloc(sizeof(node));
            pn = pn->sons[ str[i]-'a' ];
            pn->letter = str[i];
            pn->endOfWord = false;
            for(int k=0; k<26; k++) pn->sons[k] = NULL;
        }
        else {
            pn = pn->sons[ str[i]-'a' ];
        }
    }
    pn->endOfWord = true;
}

void delete_tree (node *root) {
    for(int i=0; i<26; i++) {
        if( root->sons[i] != NULL ) delete_tree(root->sons[i]);
    }
    free(root->sons);
    free(root);
}

int search_elem(char *str) {
    for(int i=0; i<strlen(str); i++) {
        if(ispunct(str[i]) || isdigit(str[i]) || isblank(str[i]) || isupper(str[i])) return 0;
    }
    node *pn = &tree;
    for(int i=0; i<strlen(str); i++) {
        if(pn->sons[ str[i]-'a' ] == NULL) return 0;
        else {
            pn = pn->sons[ str[i]-'a' ];
        }
    }
    if(pn->endOfWord == false) return 0;
    return 1;
}
