#ifndef MS_LL_INCLUDED
#define MS_LL_INCLUDED

#include "linked_list.h"

/*
		Merging strategy:
		at each iteration the lagorithm tries to insert the first element
		of the "second" chain inside the "first" one.
		The "first" one, or the main chain, is the chain that begins
		with the lowest value.
*/
list_elem *main_chain(list_elem *first, list_elem *second);

/*
		Merging strategy:
		this one works exactly like the "main chain" strategy but,
		instead of inserting a single element each time, it swaps
		the two chains.
		This way, the secondary chain becomes the main one.
*/
list_elem *swapping_chains(list_elem *first, list_elem *second);

/*
		Merging strategy:
		this strategy builds up a new singly linked list adding at
		the end the lowest of the top values of the two chains.
*/
list_elem *stream_merge(list_elem *first, list_elem *second);

/*
		An implementation of Merge-Sort algorithm for singly linked lists
		that uses "main chain" as merging strategy.
*/
list_elem *merge_sort_ll_main_chain(list_elem *list);

/*
		An implementation of Merge-Sort algorithm for singly linked lists
		that uses "swapping chains" as merging strategy.
*/
list_elem *merge_sort_ll_swap(list_elem *list);

/*
		An implementation of Merge-Sort algorithm for singly linked lists
		that uses "stream merge" as merging strategy.
*/
list_elem *merge_sort_ll_stream(list_elem *list);

#endif	// MS_LL_INCLUDED