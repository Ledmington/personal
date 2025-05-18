#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "vector.h"
#include "bit_stream.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

uint8_t bits_to_encode(const uint32_t x)
{
    if (x < 128)
    {
        return 8;
    }
    if (x < 32768)
    {
        return 16;
    }
    if (x < 8388608)
    {
        return 24;
    }
    return 32;
}

void write_encoded(write_only_bit_stream *wobs, const uint32_t x)
{
    const uint8_t b = bits_to_encode(x);
    switch (b)
    {
    case 8:
        wobs_write(wobs, true);
        for (int i = 0; i < 7; i++)
        {
            wobs_write(wobs, (x & (1 << i)) != 0);
        }
        break;
    case 16:
        wobs_write(wobs, false);
        wobs_write(wobs, true);
        for (int i = 0; i < 14; i++)
        {
            wobs_write(wobs, (x & (1 << i)) != 0);
        }
        break;
    case 24:
        wobs_write(wobs, false);
        wobs_write(wobs, false);
        wobs_write(wobs, true);
        for (int i = 0; i < 21; i++)
        {
            wobs_write(wobs, (x & (1 << i)) != 0);
        }
        break;
    case 32:
        wobs_write(wobs, false);
        wobs_write(wobs, false);
        wobs_write(wobs, false);
        wobs_write(wobs, true);
        for (int i = 0; i < 28; i++)
        {
            wobs_write(wobs, (x & (1 << i)) != 0);
        }
        break;
    default:
        fprintf(stderr, "ERROR: unexpected value %d.", b);
        exit(EXIT_FAILURE);
        break;
    }
}

void compress(const uint8_t *const __restrict__ input, const uint32_t input_bits, uint8_t **const __restrict__ output, uint32_t *const output_bits)
{
    assert(input != NULL);
    assert(output != NULL);

    read_only_bit_stream robs1 = {input, input_bits, 0};
    read_only_bit_stream robs2 = {input, input_bits, 0};

    uint32_t best_substring_start = 0;
    uint32_t best_substring_length = 0;
    vector *best_substring_occurrences = vector_create(1);
    uint32_t best_substring_bits = UINT32_MAX;

    uint32_t high = input_bits / 2;
    uint32_t low = 1;
    while (low < high)
    {
        const uint32_t substring_length = (low + high) / 2;
        printf("Testing length %d\n", substring_length);

        for (uint32_t substring_start = 0; substring_start + substring_length * max(1, vector_size(best_substring_occurrences)) < input_bits; substring_start++)
        {
            vector *occurrences = vector_create(1);

            for (uint32_t i = substring_start + substring_length; i + substring_length < input_bits; i++)
            {
                robs_set_bit_position(&robs1, substring_start);
                robs_set_bit_position(&robs2, i);

                bool equal = true;
                for (uint32_t k = 0; k < substring_length; k++)
                {
                    if (robs_next(&robs1) != robs_next(&robs2))
                    {
                        equal = false;
                        break;
                    }
                }
                if (equal)
                {
                    vector_add(occurrences, i);
                    i += substring_length;
                }
            }

            if (vector_size(occurrences) == 0)
            {
                // this substring does not repeat
                high = substring_length - 1;
                vector_destroy(occurrences);
                continue;
            }

            uint32_t new_bits = 0;
            new_bits += bits_to_encode(substring_length);         // length of A (in bits)
            new_bits += bits_to_encode(vector_size(occurrences)); // number of occurrences of A (in bits)

            // position of each of the occurrences (in bits)
            for (uint32_t k = 0; k < vector_size(occurrences); k++)
            {
                new_bits += bits_to_encode(vector_get(occurrences, k));
            }

            // length of the string without all the occurrences except one
            new_bits += (input_bits - (substring_length * vector_size(occurrences)));

            if (new_bits < best_substring_bits)
            {
                printf("Found new best start=%d; length=%d; occurrences=%d; newBits=%d\n",
                       substring_start, substring_length, vector_size(occurrences), new_bits);
                best_substring_bits = new_bits;
                best_substring_start = substring_start;
                best_substring_length = substring_length;

                // copy vector
                vector_destroy(best_substring_occurrences);
                best_substring_occurrences = occurrences;
            }
            else
            {
                vector_destroy(occurrences);
            }

            low = substring_length + 1;
        }
    }

    *output_bits = best_substring_bits;
    const uint32_t output_bytes = (best_substring_bits + 7) / 8;
    *output = (uint8_t *)malloc(output_bytes * sizeof(uint8_t));
    if (*output == NULL)
    {
        fprintf(stderr, "ERROR: could not allocate %ld bytes.\n", output_bytes * sizeof(uint8_t));
        vector_destroy(best_substring_occurrences);
        return;
    }

    read_only_bit_stream robs = {input, input_bits, 0};
    write_only_bit_stream wobs = {*output, *output_bits, 0};

    // length of A
    write_encoded(&wobs, best_substring_length);

    // number of occurrences of A
    write_encoded(&wobs, vector_size(best_substring_occurrences));

    // position of each occurrence
    for (uint32_t i = 0; i < vector_size(best_substring_occurrences); i++)
    {
        write_encoded(&wobs, vector_get(best_substring_occurrences, i));
    }

    // copy A
    robs_set_bit_position(&robs, best_substring_start);
    for (uint32_t i = 0; i < best_substring_length; i++)
    {
        wobs_write(&wobs, robs_next(&robs));
    }

    // copying portion before A
    if (best_substring_start > 0)
    {
        robs_set_bit_position(&robs, 0);
        for (uint32_t i = 0; i < best_substring_start; i++)
        {
            wobs_write(&wobs, robs_next(&robs));
        }
    }

    // copy all the other portions of the array
    uint32_t last_pos = best_substring_start + best_substring_length;
    for (uint32_t i = 0; i < vector_size(best_substring_occurrences); i++)
    {
        const uint32_t p = vector_get(best_substring_occurrences, i);
        robs_set_bit_position(&robs, last_pos);
        const uint32_t portion_length = p - last_pos;
        for (uint32_t i = 0; i < portion_length; i++)
        {
            wobs_write(&wobs, robs_next(&robs));
        }
        last_pos = p + best_substring_length;
    }

    // copy last portion
    robs_set_bit_position(&robs, last_pos);
    const uint32_t last_portion_length = input_bits - last_pos;
    for (uint32_t i = 0; i < last_portion_length; i++)
    {
        wobs_write(&wobs, robs_next(&robs));
    }

    vector_destroy(best_substring_occurrences);
}

int main()
{
    srand(time(NULL));

    const uint32_t n_bytes = 10000;
    uint32_t input_bits = n_bytes * 8;
    uint8_t *input = (uint8_t *)malloc(n_bytes * sizeof(uint8_t));
    if (input == NULL)
    {
        fprintf(stderr, "ERROR: could not allocate %ld bytes.\n", n_bytes * sizeof(uint8_t));
        return -1;
    }

    for (uint32_t i = 0; i < n_bytes; i++)
    {
        input[i] = (uint8_t)(rand() & 0x000000ff);
    }

    // printf("Input:\n");
    // for (uint32_t i = 0; i < n_bytes; i++)
    // {
    //     if (i % 16 == 0)
    //     {
    //         printf("0x%08x:", i);
    //     }
    //     printf(" %02x", input[i]);
    //     if (i % 16 == 15)
    //     {
    //         printf("\n");
    //     }
    // }
    // printf("\n");

    printf("Initial length : %d bytes (%d bits)\n", n_bytes, n_bytes * 8);
    printf("\n");

    for (int i = 0; i < 10; i++)
    {
        printf("Iteration %d\n", i);
        uint8_t *next = NULL;
        uint32_t next_bits = 0;
        compress(input, input_bits, &next, &next_bits);
        printf("New length : %d bits (%d bytes)\n\n", next_bits, (next_bits + 7) / 8);

        if (next == NULL)
        {
            fprintf(stderr, "ERROR: could not compress.\n");
            break;
        }

        free(input);
        input = next;
        input_bits = next_bits;
    }

    free(input);
    return 0;
}