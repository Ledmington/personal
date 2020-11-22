#ifndef LINKED_LIST_H_INCLUDED
#define LINKED_LIST_H_INCLUDED

typedef struct elem {
	int value;
	struct elem* next;
} list_elem;

/*
	Copies all elements of the given list inside the given array.
	The caller is responsible for allocation and deallocation of "v".
*/
void copy_list2array ( list_elem *list, int *v, const unsigned int vsize );

/*
	Creates a new singly linked list copying the content of the given array.
	A pointer to the head of the new list is returned.
*/
list_elem* copy_array2list ( int *v, const unsigned int vsize );

/*
	Prints the elements of the given list on stdout.
	Used for debug.
*/
void print_list ( list_elem *list );

/*
	Splits the given list into two halves of equal length.
	A pointer to the second half is returned.
*/
list_elem* split_list ( list_elem *list );

#endif // LINKED_LIST_H_INCLUDED