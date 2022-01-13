/*
	This program looks for non-trivial magic squares in parallel.
	A magic square is a 3x3 square
	a b c
	d e f
	g h i
	such that the sum of all elements squared in each row, column and diagonal
	is always the same.
	More formally:
	a,b,c,d,e,f,g,h,i are all positive integers
	a*a + b*b + c*c = k
	d*d + e*e + f*f = k
	g*g + h*h + i*i = k
	a*a + d*d + g*g = k
	b*b + e*e + h*h = k
	c*c + f*f + i*i = k
	a*a + e*e + i*i = k
	c*c + e*e + g*g = k

	To compile:
	gcc magicsquare.c -o magicsquare -lm -fopenmp

	To run:
	./magicsquare 1 200
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

#ifdef _WIN32
#include <windows.h>
#define clear_screen() system("cls")
#else
#define clear_screen() system("clear")
#endif

#define uint unsigned int
#define ullint unsigned long long int

typedef struct {
	// The values of the square
	uint v[9];
	// The sum of squares of the elements
	uint k;
} MagicSquare;

typedef struct {
	// The values a,b,c,d of the square
	uint v[4];
} PackedMagicSquare;

typedef struct {
	// Limits of research
	ullint start, end;

	// Index of current square being checked
	ullint i;

	// Number of square found
	ullint found;

	// Variable to check if the thread has finished to search
	bool finished;
} ThreadData;

void sleep_ms(int milliseconds)
{
    #ifdef WIN32
        Sleep(milliseconds);
    #elif _POSIX_C_SOURCE >= 199309L
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, NULL);
    #else
        usleep(milliseconds * 1000);
    #endif
}

void print_square(MagicSquare sq) {
	for(int i=0; i<3; i++) {
		for(int j=0; j<3; j++) {
			printf("%u ", sq.v[i*3+j]);
		}
		printf("\n");
	}
	printf("K: %u\n\n", sq.k);
}

uint usqrt(uint x) {
	return (uint)floor(sqrt((float)x));
}

bool issquare(uint x) {
	uint usq = usqrt(x);
	return ((x - usq*usq) == 0);
}

bool unpack(PackedMagicSquare pms, MagicSquare *sq) {
	/*
		We only keep the configurations like
		(a,b,c,d)
		such that a < b < c < d
		because the others are either trivial or
		are gonna be checked by other threads.
	*/
	if(!(pms.v[0] < pms.v[1] &&
		 pms.v[1] < pms.v[2] &&
		 pms.v[2] < pms.v[3])) {
		return 0;
	}

	uint a = pms.v[0]*pms.v[0];
	uint b = pms.v[1]*pms.v[1];
	uint c = pms.v[2]*pms.v[2];
	uint d = pms.v[3]*pms.v[3];

	uint k = a+b+c;

	if(k<=(a+d)) {
		return 0;
	}

	uint g = k-a-d; // g^2
	uint e = k-c-g; // e^2
	uint h = k-b-e; // h^2
	uint f = k-d-e; // f^2
	uint i = k-g-h; // i^2

	if(!issquare(g) ||
	   !issquare(e) ||
	   !issquare(h) ||
	   !issquare(f) ||
	   !issquare(i)) {
		return 0;
	}

	sq->v[0] = pms.v[0];
	sq->v[1] = pms.v[1];
	sq->v[2] = pms.v[2];
	sq->v[3] = pms.v[3];
	sq->v[4] = usqrt(e);
	sq->v[5] = usqrt(f);
	sq->v[6] = usqrt(g);
	sq->v[7] = usqrt(h);
	sq->v[8] = usqrt(i);
	sq->k = k;

	return 1;
}

PackedMagicSquare get_psquare(ullint n, uint low, uint high) {
	PackedMagicSquare pms;
	uint base = high-low;
	
	for(int i=3; i>=0; i--) {
		pms.v[i] = n%base + low;
		n /= base;
	}
	return pms;
}

void search(ThreadData *data, uint low, uint high) {
	MagicSquare sq;
	PackedMagicSquare pms;

	data->found = 0;
	data->finished = 0;
	for(data->i=0; data->i+data->start < data->end; data->i++){
		pms = get_psquare(data->i + data->start, low, high);
		if(unpack(pms, &sq)) {
			//print_square(sq);
			data->found++;
		}
	}
	data->finished = true;
}

int main(int argc, char *argv[]) {

	if(argc != 1 && argc != 3) {
		fprintf(stderr, "Usage: %s [low high]\n", argv[0]);
		return EXIT_FAILURE;
	}

	uint low = 1;
	uint high = 10;

	if(argc == 3) {
		low = (uint) atoi(argv[1]);
		high = (uint) atoi(argv[2]);
	}

	if(low >= high) {
		fprintf(stderr, "ERROR: low (%u) can't higher than high (%u)\n", low, high);
		return EXIT_FAILURE;
	}

	ullint limit = (ullint) pow((double)high-(double)low, 4);

	uint nthreads = omp_get_max_threads();
	ThreadData v[nthreads];
	for(int i=0; i<nthreads; i++) {
		v[i].start = limit / nthreads * i;
		v[i].end = limit / nthreads * (i+1);
		v[i].i = 0;
		v[i].found = 0;
		v[i].finished = false;
	}

	float t = omp_get_wtime();
	#pragma omp parallel num_threads(nthreads+1)
	{
		if(omp_get_thread_num() == 0) {
			while(1) {
				sleep_ms(100);
				uint count = 0;
				clear_screen();
				printf("\n");
				for(int t=0; t<nthreads; t++) {
					if(v[t].finished) {
						printf(" [%2d] %10llu checked, %10llu found (finished)\n", t, v[t].i, v[t].found);
						count++;
						continue;
					}
					printf(" [%2d] %10llu checked, %10llu found, %10llu remaining (%.3f%%)\n",
						t,
						v[t].i,
						v[t].found,
						(v[t].end-v[t].start+1)-v[t].i,
						(double)v[t].i/(double)(v[t].end-v[t].start+1)*100);
				}
				if(count == nthreads) break;
			}
		}
		else {
			search(&v[omp_get_thread_num()-1], low, high);
		}
	}
	t = omp_get_wtime() - t;

	ullint found = 0;
	for(int i=0; i<nthreads; i++) {
		found += v[i].found;
	}

	clear_screen();
	printf("\n ### Report ###\n"
		   " %u threads used\n"
		   " %.3f seconds of execution\n"
		   " %llu total squares checked\n"
		   " %llu actual magic squares found (%.3e%%)\n\n",
		   nthreads,
		   (double)t,
		   limit,
		   found,
		   (double)found/(double)limit*100);

	return EXIT_SUCCESS;
}
