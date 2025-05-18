#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "bit_stream.h"

bool robs_has_next(const read_only_bit_stream *const robs)
{
    assert(robs != NULL);
    return robs->bit_index < robs->n_bits;
}

bool robs_next(read_only_bit_stream *const robs)
{
    assert(robs != NULL);
    assert(robs_has_next(robs));
    const uint8_t byte = robs->data[robs->bit_index >> 3];
    const bool bit = byte & (1 << (7 - (robs->bit_index & 0x00000007)));
    robs->bit_index++;
    return bit;
}

void robs_set_bit_position(read_only_bit_stream *const robs, const uint32_t bit_position)
{
    assert(robs != NULL);
    assert(robs->data != NULL);
    assert(bit_position < robs->n_bits);
    robs->bit_index = bit_position;
}

void wobs_write(write_only_bit_stream *wobs, const bool bit)
{
    assert(wobs != NULL);
    assert(wobs->data != NULL);
    assert(wobs->bit_index < wobs->n_bits);
    if (bit)
    {
        wobs->data[wobs->bit_index >> 3] |= (1 << (7 - (wobs->bit_index & 0x00000007)));
    }
    wobs->bit_index++;
}