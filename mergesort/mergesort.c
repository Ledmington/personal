/*
	Performance comparison between two types of mergesort:
	 - the first one is the classic one done on a contiguous array
	    and a temporary array allocation for each 'merge' call
	 - the second one is the classic one done with a contiguous array
	    and a single temporary array
	 - the third one is its version done on a linked list

	Compile with:
	gcc -Wall -Wpdantic *.c -o mergesort.exe

	Run with:
	./mergesort.exe [n]

	Example:
	./mergesort 8192000
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "linked_list.h"
#include "ms_ll.h"
#include "ms_array.h"

void swap (int *a, int *b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

int* init ( const unsigned int n ) {
	int *v = (int*) malloc(n * sizeof(int));
	assert(v);

	for (unsigned int i=0; i<n; i++) {
		v[i] = i;
	}
	return v;
}

void shuffle ( int* v, const unsigned int n ) {
	unsigned int j;
	for (unsigned int i=0; i<n; i++) {
		j = rand()%n;
		swap (&v[i], &v[j]);
	}
}

void check ( int* v, const unsigned int n ) {
	for (unsigned int i=0; i<n; i++) {
		if (v[i] != i) {
			printf("v[%d] = %d, expected %d\n", i, v[i], i);
			return;
		}
	}
	printf("Check OK\n");
}

list_elem* init_list ( const unsigned int n ) {
	list_elem* head = NULL;
	list_elem *tmp;
	for (unsigned int i=0; i<n; i++) {
		tmp = (list_elem*) malloc(sizeof(list_elem));
		assert(tmp);
		tmp->value = n - 1 - i;
		tmp->next = head;
		head = tmp;
	}
	return head;
}

list_elem* shuffle_list ( list_elem *list, const unsigned int n ) {
	int *tmp = (int*) malloc (n * sizeof(int));
	assert(tmp);

	list_elem *head = list;
	copy_list2array (head, tmp, n);

	shuffle (tmp, n);

	head = list;
	unsigned int i = 0;
	while (head != NULL) {
		head->value = tmp[i];
		head = head->next;
		i++;
	}
	free(tmp);
	return list;
}

void check_list ( list_elem *list, const unsigned int n ) {
	list_elem *head = list;
	for (unsigned int i=0; i<n; i++) {
		if (head->value != i) {
			printf("list[%u] = %d, expected %d\n", i, head->value, i);
			return;
		}
		head = head->next;
	}
	printf("Check OK\n");
}

void free_list ( list_elem* list ) {
	list_elem *tmp = list;
	while (list != NULL) {
		tmp = list->next;
		free(list);
		list = tmp;
	}
}

int main ( int argc, char *argv[] ) {
	srand(time(NULL));
	int *v, *tmp;
	list_elem *list;
	unsigned int n = 128*1024;
	const unsigned int max_n = 256*1024*1024;
	clock_t t;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [len]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		n = atoi(argv[1]);
	}

	if (n > max_n) {
		fprintf(stderr, "n is too large. Max value is %u\n", max_n);
		return EXIT_FAILURE;
	}

	printf("Explanation:\n");
	printf("==ARRAY==\n");
	printf(" - \'no tmp\' allocates a new tiny vector each time it needs to merge two arrays\n");
	printf(" - \'tmp\' receives a single giant vector as input and uses it everytime\n");
	printf("==SINGLY LINKED LIST==\n");
	printf(" - \'main chain\' merges one chain into the other\n");
	printf(" - \'swapping chains\' swaps pointers to the chains, instead of inserting one element at a time\n");
	printf(" - \'stream merging\' builds a new chain one element at a time\n\n");

	v = init(n);

	printf("Sorting %u elements...\n", n);

	shuffle(v, n);

	t = clock();
	ms_no_tmp(v, 0, n-1);
	t = clock() - t;
	printf("No tmp: %.3f seconds\n", (double)t/CLOCKS_PER_SEC);

	check(v, n);

	shuffle(v, n);
	tmp = (int*) malloc(n * sizeof(int));
	assert(tmp);

	t = clock();
	ms_tmp(v, tmp, 0, n-1);
	t = clock() - t;
	printf("Tmp: %.3f seconds\n", (double)t/CLOCKS_PER_SEC);

	check(v, n);

	free(v);
	free(tmp);

	list = init_list(n);
	list = shuffle_list(list, n);

	t = clock();
	list = merge_sort_ll_main_chain(list);
	t = clock() - t;
	printf("Main chain: %.3f seconds\n", (double)t/CLOCKS_PER_SEC);

	check_list(list, n);

	free_list(list);

	list = init_list(n);
	list = shuffle_list(list, n);

	t = clock();
	list = merge_sort_ll_swap(list);
	t = clock() - t;
	printf("Swapping chains: %.3f seconds\n", (double)t/CLOCKS_PER_SEC);

	check_list(list, n);

	free_list(list);

	list = init_list(n);
	list = shuffle_list(list, n);

	t = clock();
	list = merge_sort_ll_stream(list);
	t = clock() - t;
	printf("Stream merging: %.3f seconds\n", (double)t/CLOCKS_PER_SEC);

	check_list(list, n);

	free_list(list);

	return EXIT_SUCCESS;
}