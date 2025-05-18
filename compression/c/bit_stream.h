#pragma once

#include <stdint.h>

typedef struct
{
    const uint8_t *const data;
    const uint32_t n_bits;
    uint32_t bit_index;
} read_only_bit_stream;

bool robs_next(read_only_bit_stream *const robs);
void robs_set_bit_position(read_only_bit_stream *const robs, const uint32_t bit_position);

typedef struct
{
    uint8_t *data;
    const uint32_t n_bits;
    uint32_t bit_index;
} write_only_bit_stream;

void wobs_write(write_only_bit_stream *wobs, const bool bit);
