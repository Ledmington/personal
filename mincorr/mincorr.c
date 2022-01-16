/*
	This program finds the array with minimum correlation with a given array.

	To compile:
	gcc mincorr.c -o mincorr -lm

	To run:
	./mincorr
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

double randab(double a, double b) {
	return (double)rand() / (double)RAND_MAX * (b-a) + a;
}

void init_random(double *v, unsigned int n) {
	for(unsigned int i=0; i<n; i++) {
		v[i] = randab(-1, 1);
	}
}

void copy(double *src, double *dst, unsigned int n) {
	for(unsigned int i=0; i<n; i++) {
		dst[i] = src[i];
	}
}

void print(double *v, unsigned int n) {
	printf("[");
	for(unsigned int i=0; i<n-1; i++) {
		printf("%f ", v[i]);
	}
	printf("%f]\n", v[n-1]);
}

double mean(double *v, unsigned int n) {
	double sum = 0;
	for(unsigned int i=0; i<n; i++) {
		sum += v[i];
	}
	return sum / (double)n;
}

double stddev(double *v, unsigned int n) {
	double m = mean(v, n);
	double sum = 0;
	for(unsigned int i=0; i<n; i++) {
		sum += (v[i]-m)*(v[i]-m);
	}
	return sqrt(sum / (double)n);
}

double covariance(double *x, double *y, unsigned int n) {
	double xm = mean(x, n);
	double ym = mean(y, n);
	double sum = 0;
	for(unsigned int i=0; i<n; i++) {
		sum += (x[i]-xm)*(y[i]-ym);
	}
	return sum / (double)n;
}

// Pearson correlation
double corr(double *x, double *y, unsigned int n) {
	return (covariance(x, y, n)) / (stddev(x, n) * stddev(y, n));
}

int main(void) {
	srand(time(NULL));

	unsigned int n = 10;
	double v[n];
	for(unsigned int i=0; i<n; i++) {
		v[i] = (double)i;
	}

	double tmp[n];
	double best[n];
	double best_score = 1;

	unsigned int maxiterations = 123456789;

	for(unsigned int it=0; it<maxiterations && best_score>0; it++) {
		init_random(tmp, n);
		double score = fabs(corr(tmp, v, n));
		if(score < best_score) {
			best_score = score;
			copy(tmp, best, n);
			printf("It.%d:\n", it);
			print(best, n);
			printf("New correlation: %e\n\n", score);
		}
	}

	return EXIT_SUCCESS;
}