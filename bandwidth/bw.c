#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined(__clang__)
#define COMPILER "clang"
#define COMPILER_MAJOR __clang_major__
#define COMPILER_MINOR __clang_minor__
#define COMPILER_PATCH __clang_patchlevel__
#elif defined(__GNUG__) || defined(__GNUC__)
#define COMPILER "gcc"
#define COMPILER_MAJOR __GNUC__
#define COMPILER_MINOR __GNUC_MINOR__
#define COMPILER_PATCH __GNUC_PATCHLEVEL__
#else
#define COMPILER "unknown"
#define COMPILER_MAJOR 0
#define COMPILER_MINOR 0
#define COMPILER_PATCH 0
#endif

#define RESET "\033[0m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

#define DATA_TYPE double
#define INDEX_TYPE uint32_t
#define RESULT_TYPE double

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

RESULT_TYPE mean(const RESULT_TYPE *values, const size_t length) {
	RESULT_TYPE s = (RESULT_TYPE)0;
	for (INDEX_TYPE i = 0; i < length; i++) {
		s += values[i];
	}
	return s / (RESULT_TYPE)length;
}

RESULT_TYPE stddev(const RESULT_TYPE *values, const size_t length,
				   const RESULT_TYPE m) {
	RESULT_TYPE s = (RESULT_TYPE)0;
	for (INDEX_TYPE i = 0; i < length; i++) {
		s += (values[i] - m) * (values[i] - m);
	}
	return sqrt(s / (RESULT_TYPE)length);
}

RESULT_TYPE format_bytes(RESULT_TYPE b) {
	while (b >= 1024) {
		b /= 1024;
	}
	return b;
}

const char *biggest_byte_unit_for(RESULT_TYPE b) {
	RESULT_TYPE bytes = b;
	if (bytes < 1024) {
		return "B";
	}
	bytes /= 1024;
	if (bytes < 1024) {
		return "KB";
	}
	bytes /= 1024;
	if (bytes < 1024) {
		return "MB";
	}
	bytes /= 1024;
	if (bytes < 1024) {
		return "GB";
	}
	return "TB";
}

DATA_TYPE randab(const DATA_TYPE a, const DATA_TYPE b) {
	return (DATA_TYPE)rand() / (DATA_TYPE)RAND_MAX * (b - a) + a;
}

// returns time in seconds
double timer() {
	struct timeval tp;
	struct timezone tzp;

	gettimeofday(&tp, &tzp);
	return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

int timer_precision() {
	double times[1000];

	for (int i = 0; i < 1000; i++) {
		double t1 = timer();
		while (timer() == t1) {
		}
		times[i] = timer() - t1;
	}

	double min = 2e9;
	for (int i = 0; i < 1000; i++) {
		min = MIN(min, times[i]);
	}

	return (int)(min * 1e9);
}

int main(const int argc, const char **argv) {
	srand(time(NULL));

	uint32_t max_bytes = 1 << 30;  // 1 GB by default

	if (argc > 1) {
		max_bytes = strtoul(argv[1], NULL, 10);
	}
	if (argc > 2) {
		fprintf(
			stdout,
			"I do not need more than one argument. I'll ignore the others.\n");
	}

	const uint32_t max_length = max_bytes / sizeof(DATA_TYPE);

	fprintf(stdout, " --- Bandiwidth benchmark --- \n");
	fprintf(stdout, "Compiled with %s %d.%d.%d on %s %s\n", COMPILER,
			COMPILER_MAJOR, COMPILER_MINOR, COMPILER_PATCH, __DATE__, __TIME__);
	fprintf(stdout, "------------------------------\n");

	fprintf(stdout, "Size of a single data element: %ld bits\n",
			8 * sizeof(DATA_TYPE));
	fprintf(stdout, "Size of an index: %ld bits\n", 8 * sizeof(INDEX_TYPE));
	fprintf(stdout, "Max elements per array: %d\n", max_length);
	fprintf(stdout, "I will use 2 arrays\n");
	fprintf(stdout, "Timer precision: %d nanoseconds\n", timer_precision());
#ifdef _OPENMP
	int nthreads_detected = 0;
#pragma omp parallel
	{
#pragma omp master
		{ nthreads_detected = omp_get_num_threads(); }
	}
	fprintf(stdout, "Threads detected: %d\n", nthreads_detected);
	int nthreads_counted = 0;
#pragma omp parallel
	{
#pragma omp atomic
		nthreads_counted++;
	}
	fprintf(stdout, "Threads counted: %d\n", nthreads_counted);
#endif
	fprintf(stdout, "\n");

	for (uint32_t bytes = sizeof(DATA_TYPE); bytes <= max_bytes; bytes *= 2) {
		const uint32_t length = bytes / sizeof(DATA_TYPE);

#ifdef _OPENMP
		// make sure that there is at least one element for each thread
		if (length < nthreads_detected) {
			continue;
		}
#endif

		DATA_TYPE *a = (DATA_TYPE *)malloc(length * sizeof(DATA_TYPE));
		if (a == NULL) {
			fprintf(stderr, "ERROR: failed to allocate %ld bytes\n",
					length * sizeof(DATA_TYPE));
			break;
		}
		DATA_TYPE *b = (DATA_TYPE *)malloc(length * sizeof(DATA_TYPE));
		if (a == NULL) {
			fprintf(stderr, "ERROR: failed to allocate %ld bytes\n",
					length * sizeof(DATA_TYPE));
			free(a);
			break;
		}

		int values_size = 100;
		int values_length = 0;
		RESULT_TYPE *values =
			(RESULT_TYPE *)malloc(values_size * sizeof(RESULT_TYPE));
		if (values == NULL) {
			fprintf(stderr, "ERROR: failed to allocate %ld bytes\n",
					values_size * sizeof(RESULT_TYPE));
			free(a);
			free(b);
			break;
		}

		double mean_bw = 0.0;  // mean value of bandwidth
		double sd_bw = 0.0;	   // stddev of the bandwidth
		double hwci_bw =
			0.0;			// half width confidence interval of the bandwidth
		size_t inside = 0;	// number of values inside the 2*stddev range

		do {
			// init vectors
			for (INDEX_TYPE j = 0; j < length; ++j) {
				a[j] = randab((DATA_TYPE)0, (DATA_TYPE)1);
				b[j] = randab((DATA_TYPE)0, (DATA_TYPE)1);
			}

			const double start = timer();

#pragma omp parallel for default(none) shared(a, b) firstprivate(length) \
	schedule(static)
			// copy a into b
			for (INDEX_TYPE i = 0; i < length; i++) {
				b[i] = a[i];
			}

			const double finish = timer();
			const double elapsed = finish - start;

			/*
				Computing bandwidth as bytes transferred per second.
				The 2 * bytes is there because we are copying values from one
				array to another: 1 copy = 1 read + 1 write
			*/
			const double new_value = (2 * bytes) / elapsed;
			if (values_length >= values_size) {
				values_size *= 2;
				values = realloc(values, values_size * sizeof(RESULT_TYPE));
				if (values == NULL) {
					fprintf(stderr, "ERROR: failed to allocate %ld bytes\n",
							values_size * sizeof(RESULT_TYPE));
					free(a);
					free(b);
					break;
				}
			}
			values[values_length++] = new_value;

			// check a == b
			for (INDEX_TYPE j = 0; j < length; j++) {
				if (a[j] != b[j]) {
					fprintf(stderr,
							"ERROR: arrays differ at index %d: %f; %f\n", j,
							a[j], b[j]);
					break;
				}
			}

			mean_bw = mean(values, values_length);
			sd_bw = stddev(values, values_length, mean_bw);
			hwci_bw = 2 * sd_bw / sqrt(values_length);

			inside = 0;
			// count how many values are inside the 2 stddev range
			for (size_t j = 0; j < values_length; j++) {
				if (values[j] <= (mean_bw + 2 * sd_bw) &&
					values[j] >= (mean_bw - 2 * sd_bw)) {
					inside++;
				}
			}
		} while (
			// must run at least one time
			values_length < 1 ||
			// must have at least one value inside and one outside the 2 stddev
			// range
			values_length == inside ||
			// the confidence interval must not go on negative values
			hwci_bw >= mean_bw ||
			// at least 0.9545 of the values must be inside 2 standard
			// deviations
			((double)(inside) / (double)(values_length)) < 0.9545);

		free(a);
		free(b);
		free(values);

		printf(CYAN "%7.3f %2s" WHITE " (%12u elements): " GREEN
					"%7.3f %2s/s" WHITE " +- " YELLOW "%7.3f %2s/s" RESET "\n",
			   format_bytes(bytes), biggest_byte_unit_for(bytes), length,
			   format_bytes(mean_bw), biggest_byte_unit_for(mean_bw),
			   format_bytes(hwci_bw), biggest_byte_unit_for(hwci_bw));
	}

	return 0;
}