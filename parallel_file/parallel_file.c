/*
	parallel_file.c

	Showcase of the best ways to read a file in parallel.

	Compile with:
	gcc -Wall -Werror -Wpedantic -Ofast parallel_file.c -o parallel_file
   -fopenmp

	Run with:
	./parallel_file
*/

#include <assert.h>
#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define min(a, b) (a < b ? a : b)

#define profile(code)                        \
	do {                                     \
		t = omp_get_wtime();                 \
		code;                                \
		t = omp_get_wtime() - t;             \
		printf("%.3f seconds elapsed\n", t); \
	} while (0)

void generate_file(const char *filename, const long unsigned int n) {
	FILE *f = fopen(filename, "w");
	for (long unsigned int i = 0; i < n; i++) {
		fprintf(f, "%010u\n", rand());
	}
	fclose(f);
}

long unsigned int get_file_size(const char *filename) {
	FILE *f = fopen(filename, "r");
	fseek(f, 0L, SEEK_END);
	const long unsigned int sz = ftell(f);
	fclose(f);
	return sz;
}

unsigned int read_serially(const char *filename) {
	unsigned int best = UINT_MAX;
	FILE *f = fopen(filename, "r");
	unsigned int n;
	while (1 == fscanf(f, "%u\n", &n)) {
		best = min(best, n);
	}
	fclose(f);
	return best;
}

unsigned int partition_logically(const char *filename) {
	unsigned int best = UINT_MAX;
	const long unsigned int length = get_file_size(filename);
#pragma omp parallel default(none) shared(filename, length) reduction(min \
																	  : best)
	{
		FILE *f = fopen(filename, "r");
		const unsigned int idx = omp_get_thread_num();
		const long unsigned int portion = length / omp_get_num_threads();
		const long unsigned int start = portion * idx;
		fseek(f, start, SEEK_SET);
		unsigned int n;
		for (long unsigned int i = start; i < start + portion; i++) {
			fscanf(f, "%u\n", &n);
			best = min(best, n);
		}
		fclose(f);
	}
	return best;
}

unsigned int copy_and_partition(const char *filename) {
	unsigned int best = UINT_MAX;
	const unsigned int nth = omp_get_max_threads();
	const long unsigned int length = get_file_size(filename);
	char copies_names[nth][11];
	FILE *copies[nth];
	FILE *main = fopen(filename, "r");

	// Creating copies
	for (unsigned int i = 0; i < nth; i++) {
		sprintf(copies_names[i], "tmp%02u.txt", i);
		copies[i] = fopen(copies_names[i], "w");
	}

	// Copying content
	while (!feof(main)) {
		char ch = fgetc(main);
		for (unsigned int i = 0; i < nth; i++) {
			fputc(ch, copies[i]);
		}
	}

	// Closing files
	for (unsigned int i = 0; i < nth; i++) {
		fclose(copies[i]);
	}
	fclose(main);

#pragma omp parallel default(none) shared(length, copies, copies_names) \
	reduction(min                                                       \
			  : best)
	{
		const unsigned int idx = omp_get_thread_num();
		copies[idx] = fopen(copies_names[idx], "r");
		const long unsigned int portion = length / omp_get_num_threads();
		const long unsigned int start = portion * idx;
		fseek(copies[idx], start, SEEK_SET);
		unsigned int n;
		for (long unsigned int i = start; i < start + portion; i++) {
			fscanf(copies[idx], "%u\n", &n);
			best = min(best, n);
		}
		fclose(copies[idx]);
	}
	return best;
}

unsigned int split_and_read(const char *filename) {
	unsigned int best = UINT_MAX;
	const unsigned int nth = omp_get_max_threads();
	char copies_names[nth][11];
	FILE *copies[nth];
	FILE *main = fopen(filename, "r");

	// Creating copies
	for (unsigned int i = 0; i < nth; i++) {
		sprintf(copies_names[i], "tmp%02u.txt", i);
		copies[i] = fopen(copies_names[i], "w");
	}

	// Copying content
	long unsigned int line = 0;
	unsigned int n;
	while (1 == fscanf(main, "%u\n", &n)) {
		fprintf(copies[line % nth], "%010u\n", n);
		line++;
	}

	// Closing files
	for (unsigned int i = 0; i < nth; i++) {
		fclose(copies[i]);
	}
	fclose(main);

#pragma omp parallel default(none) shared(copies, copies_names) \
	reduction(min                                               \
			  : best)
	{
		const unsigned int idx = omp_get_thread_num();
		copies[idx] = fopen(copies_names[idx], "r");
		unsigned int n;
		while (1 == fscanf(copies[idx], "%u\n", &n)) {
			best = min(best, n);
		}
		fclose(copies[idx]);
	}
	return best;
}

int main(void) {
	srand(time(NULL));

	const long unsigned int n = 100 * 1000 * 1000;
	const unsigned int max_threads = omp_get_max_threads();
	const char *filename = "tmp.txt";
	double t;
	unsigned int result;

	printf("%u available threads\n\n", max_threads);

	printf("Generating a file with %1.2e numbers\n", (double)n);
	profile(generate_file(filename, n));
	const long unsigned int filesize = get_file_size(filename);
	printf("Actual size: %lu bytes\n\n", filesize);

	printf("Reading serially\n");
	profile(result = read_serially(filename));
	printf("Result: %u\n\n", result);

	printf("Partitioning logically\n");
	profile(result = partition_logically(filename));
	printf("Result: %u\n\n", result);

	printf("Physical copies partitioned logically\n");
	profile(result = copy_and_partition(filename));
	printf("Result: %u\n\n", result);

	printf("Physical copies\n");
	profile(result = split_and_read(filename));
	printf("Result: %u\n\n", result);

	return EXIT_SUCCESS;
}