#ifndef CQFS_H_INCLUDED
#define CQFS_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef CQFS_ELEMENT_TYPE
#define CQFS_ELEMENT_TYPE double
#endif

typedef struct
{
    CQFS_ELEMENT_TYPE *queue;

    // how much is allocated
    uint32_t size;

    // how much is actually filled
    uint32_t length;

    // where is the first element (where to pop from)
    uint32_t first_element_index;
} CQFS; // stands for "Circular Queue with Fixed Size"

/*
Initializes the given Circular Queue with Fixed Size
using the given size.
*/
void cqfs_init(CQFS *cqfs, const uint32_t size)
{
    assert(size > 0);
    assert(cqfs != NULL);
    cqfs->queue = (CQFS_ELEMENT_TYPE *)malloc(size * sizeof(CQFS_ELEMENT_TYPE));
    assert(cqfs->queue != NULL);
    cqfs->size = size;
    cqfs->length = 0;
    cqfs->first_element_index = 0;
}

/*
Frees the memory allocated by the given CQFS.
*/
void cqfs_destroy(CQFS *cqfs)
{
    assert(cqfs != NULL);
    free(cqfs->queue);
    cqfs->size = 0;
}

/*
Returns the max size that has been allocated.
*/
uint32_t cqfs_get_size(const CQFS *cqfs)
{
    assert(cqfs != NULL);
    return cqfs->size;
}

/*
Returns the actual number of elements currently present.
*/
uint32_t cqfs_get_length(const CQFS *cqfs)
{
    assert(cqfs != NULL);
    return cqfs->length;
}

bool cqfs_is_empty(const CQFS *cqfs)
{
    assert(cqfs != NULL);
    return cqfs->length == 0;
}

bool cqfs_is_full(const CQFS *cqfs)
{
    assert(cqfs != NULL);
    return cqfs->size == cqfs->length;
}

/*
Pushes an element into the queue.
*/
void cqfs_push(CQFS *cqfs, const CQFS_ELEMENT_TYPE element)
{
    assert(cqfs != NULL);
    assert(cqfs->queue != NULL);
    cqfs->queue[(cqfs->first_element_index + cqfs->length) % cqfs->size] = element;
    cqfs->length++;
}

/*
Pops (removes and returns) an element from the queue.
*/
CQFS_ELEMENT_TYPE cqfs_pop(CQFS *cqfs)
{
    assert(cqfs != NULL);
    assert(cqfs->queue != NULL);
    CQFS_ELEMENT_TYPE result = cqfs->queue[cqfs->first_element_index];
    cqfs->first_element_index = (cqfs->first_element_index + 1) % cqfs->size;
    cqfs->length--;
    return result;
}

#endif // CQFS_H_INCLUDED