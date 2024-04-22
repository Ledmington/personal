#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cqfs.h"

double randab(const double a, const double b) {
	return (double)rand() / (double)RAND_MAX * (b - a) + a;
}

int32_t main(void) {
	srand(time(NULL));

	const uint32_t max_iterations = 10 * 1000 * 1000;
	const uint32_t k = 10 * 1000;

	fprintf(stdout, "Creating a Circular Queue with Fixed Size of %u\n", k);
	CQFS cqfs;
	cqfs_init(&cqfs, k);

	clock_t t;

	uint32_t insertions = 0;
	uint32_t deletions = 0;
	fprintf(stdout, "Starting benchmark with %u iterations\n", max_iterations);

	t = clock();
	for (uint32_t i = 0; i < max_iterations; i++) {
		if (randab(0.0f, 1.0f) < 0.5f && !cqfs_is_full(&cqfs)) {
			cqfs_push(&cqfs, randab(0.0f, 100.0f));
			insertions++;
		} else {
			if (!cqfs_is_empty(&cqfs)) {
				cqfs_pop(&cqfs);
				deletions++;
			}
		}
	}
	t = clock() - t;

	const double elapsed = (double)t / (double)CLOCKS_PER_SEC;

	fprintf(stdout, "Total elapsed time: %2.6f seconds\n", elapsed);
	fprintf(stdout, "Time per iteration: %2.9f seconds\n",
			elapsed / (double)max_iterations);
	fprintf(stdout, "%u insertions\n", insertions);
	fprintf(stdout, "%u deletions\n", deletions);

	cqfs_destroy(&cqfs);

	return EXIT_SUCCESS;
}