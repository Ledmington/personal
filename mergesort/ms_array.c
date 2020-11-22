#include "ms_array.h"

#include <stdlib.h>
#include <assert.h>

void merge_no_tmp (int* v, const unsigned int start, const unsigned int middle, const unsigned int end) {

	// Allocating temporary array
	int* tmp = (int*) malloc((end-start+1) * sizeof(int));
	assert(tmp);

	unsigned int pos1 = start;
	unsigned int pos2 = middle+1;
	unsigned int index = 0;

	// Actual merging
	while(pos1<=middle && pos2<=end){
		if(v[pos1] <= v[pos2]){
			tmp[index] = v[pos1];
			pos1++;
		}
		else{
			tmp[index] = v[pos2];
			pos2++;
		}
		index++;
	}

	// Copying the remaining elements from the first half
	while(pos1<=middle){
		tmp[index] = v[pos1];
		index++;
		pos1++;
	}

	// Copying the remaining elements from the second half
	while(pos2<=end){
		tmp[index] = v[pos2];
		index++;
		pos2++;
	}

	// Copy-back into main array
	for(index=start; index<=end; index++){
		v[index] = tmp[index-start];
	}

	free(tmp);
}

void ms_no_tmp (int* v, const unsigned int start, const unsigned int end) {
  if(start < end) {

	unsigned int middle = (start + end)/2;

	ms_no_tmp(v, start, middle);
	ms_no_tmp(v, middle+1, end);

	merge_no_tmp(v, start, middle, end);
  }
}

void merge_tmp (int *v, int *tmp, const unsigned int start, const unsigned int middle, const unsigned int end) {

	unsigned int pos1 = start;
	unsigned int pos2 = middle+1;
	unsigned int index = start;

	// Merging
	while(pos1<=middle && pos2<=end){
		if(v[pos1] <= v[pos2]){
			tmp[index] = v[pos1];
			pos1++;
		}
		else{
			tmp[index] = v[pos2];
			pos2++;
		}
		index++;
	}

	// Copying the remaining elements from the first half
	while(pos1<=middle){
		tmp[index] = v[pos1];
		index++;
		pos1++;
	}

	// Copying the remaining elements from the second half
	while(pos2<=end){
		tmp[index] = v[pos2];
		index++;
		pos2++;
	}

	// Copy-back into main array
	for(index=start; index<=end; index++){
		v[index] = tmp[index];
	}
}

void ms_tmp (int *v, int *tmp, const unsigned int start, const unsigned int end) {
	if(start < end){

	unsigned int middle = (start + end)/2;

	ms_tmp(v, tmp, start, middle);
	ms_tmp(v, tmp, middle+1, end);

	merge_tmp(v, tmp, start, middle, end);
  }
}