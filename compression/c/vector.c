#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

vector *vector_create(const uint32_t initial_capacity)
{
    assert(initial_capacity > 0);
    vector *v = (vector *)malloc(sizeof(vector));
    if (v == NULL)
    {
        fprintf(stderr, "ERROR: unable to allocate %ld bytes.\n", sizeof(vector));
        exit(EXIT_FAILURE);
    }

    v->data = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));
    if (v->data == NULL)
    {
        free(v);
        fprintf(stderr, "ERROR: unable to allocate %ld bytes.\n", initial_capacity * sizeof(uint32_t));
        exit(EXIT_FAILURE);
    }

    v->capacity = initial_capacity;
    v->size = 0;

    return v;
}

void vector_destroy(vector *vector)
{
    if (vector != NULL)
    {
        if (vector->data != NULL)
        {
            free(vector->data);
        }
        free(vector);
    }
}

uint32_t vector_size(const vector *const vector)
{
    assert(vector != NULL);
    return vector->size;
}

void vector_add(vector *vector, const uint32_t elem)
{
    assert(vector != NULL);
    assert(vector->data != NULL);
    if (vector->size >= vector->capacity)
    {
        uint32_t *tmp = (uint32_t *)malloc(vector->capacity * 2 * sizeof(uint32_t));
        if (tmp == NULL)
        {
            fprintf(stderr, "ERROR: unable to allocate %ld bytes.\n", vector->capacity * 2 * sizeof(uint32_t));
            free(vector->data);
            free(vector);
            exit(EXIT_FAILURE);
        }
        memcpy(tmp, vector->data, vector->capacity * sizeof(uint32_t));
        free(vector->data);
        vector->data = tmp;
        vector->capacity *= 2;
    }
    vector->data[vector->size] = elem;
    vector->size++;
}

uint32_t vector_get(const vector *const vector, const uint32_t index)
{
    assert(vector != NULL);
    assert(vector->data != NULL);
    assert(index < vector->size);
    return vector->data[index];
}