/*
    parallel_file.c

    Showcase of the best ways to read a file in parallel.

    Compile with:
    gcc -Wall -Werror -Wpedantic -O3 parallel_file.c -o parallel_file

    Run with:
    ./parallel_file
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

#define min(a,b) (a<b?a:b)

long unsigned int get_file_size(const char* filename) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    const long unsigned int sz = ftell(f);
    fclose(f);
    return sz;
}

unsigned int read_serially(const char *filename) {
    unsigned int best = UINT_MAX;
    FILE *f = fopen(filename, "r");
    char *line;
    int length;
    while(!feof(f)) {
        assert(getline(&line, (size_t*)&length, f) != -1);
        best = min(best, (unsigned int)atoi(line));
    }
    fclose(f);
    return best;
}

int main(void) {
    srand(time(NULL));

    const long unsigned int n = 100 * 1000 * 1000;
    const char* filename = "tmp.txt";
    FILE *f;
    clock_t t;
    unsigned int result;

    printf("Generating a file with %1.2e numbers\n", (double)n);
    t = clock();
    f = fopen(filename, "w");
    for(long unsigned int i=0; i<n; i++) {
        fprintf(f, "%d\n", rand());
    }
    fclose(f);
    t = clock() - t;
    printf("%.3f seconds elapsed\n", (float)t / (float)CLOCKS_PER_SEC);
    printf("Actual size: %lu bytes\n\n", get_file_size(filename));

    printf("Reading serially\n");
    t = clock();
    result = read_serially(filename);
    t = clock() - t;
    printf("%.3f seconds elapsed\n", (float)t / (float)CLOCKS_PER_SEC);
    printf("Result: %u\n\n", result);

    return EXIT_SUCCESS;
}