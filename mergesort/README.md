# Merge-sort performance evaluation

Performance comparison between five types of mergesort:
- the classic one done on a contiguous array and a temporary array allocation for each 'merge' call
- the classic one done with a contiguous array and a single temporary array
- merge-sort on a singly linked list with "main chain" merging strategy: each element of the "second list" is inserted inside the "first" (or "main") one
- merge-sort on a singly linked list with "swapping chains" merging strategy: instead, of inserting one element at a time, the pointers to the two chains are swapped
- merge-sort on a singly linked list with "stream merge" merging strategy: a third list is built up one element at a time from the given two.

**Compile** with:
`gcc -Wall -Wpdantic *.c -o mergesort.exe`

**Run** with:
`./mergesort.exe [n]`

Example:
`./mergesort.exe 8192000`
