#ifndef MS_ARRAY_H_INCLUDED
#define MS_ARRAY_H_INCLUDED

/*
	Merges two sorted halves of a given array without using a given temporary array.
	Therefore, a new temporary array is allocated for each call to this function.
*/
void merge_no_tmp (int* v, const unsigned int start, const unsigned int middle, const unsigned int end);

/*
	An implementation of the Merge-Sort algoirthm that doesn't use a given temporary array.
*/
void ms_no_tmp (int* v, const unsigned int start, const unsigned int end);

/*
	Merges two sorted halves of a given array using the temporary array "tmp".
*/
void merge_tmp (int *v, int *tmp, const unsigned int start, const unsigned int middle, const unsigned int end);

/*
	An implementation of the Merge-Sort algorithm that requires a temporary array.
	The caller is responsible for allocation and deallocation of "tmp".
*/
void ms_tmp (int *v, int *tmp, const unsigned int start, const unsigned int end);

#endif // MS_ARRAY_H_INCLUDED