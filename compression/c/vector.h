#pragma once

typedef struct
{
    uint32_t *data;
    uint32_t capacity;
    uint32_t size;
} vector;

vector *vector_create(const uint32_t initial_capacity);
void vector_destroy(vector *vector);
uint32_t vector_size(const vector *const vector);
void vector_add(vector *vector, const uint32_t elem);
uint32_t vector_get(const vector *const vector, const uint32_t index);
