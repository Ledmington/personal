/*
    Implementation and comparison of:
    - serial N^2
    - parallel N^2
    - serial quad-tree
    - parallel quad-tree

    Compile with:
    gcc -Wall -Wpedantic -std=c99 -fopenmp quad_tree.c -o quad_tree

    Run with:
    ./quad_tree [n]
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

#define min(a,b) (a<b)?a:b
#define max(a,b) (a>b)?a:b

// Domain borders
#define MIN_X 0.0f
#define MAX_X 50.0f
#define MIN_Y 0.0f
#define MAX_Y 50.0f

#define MIN_RADIUS 0.0f
#define MAX_RADIUS 1.0f

// Quad-tree parameters
const int MAX_CIRCLES = 1000;  // Max number of circles in each cell
const float size_x = 2.1f * (float)MAX_RADIUS;   // x-size of each cell
const float size_y = 2.1f * (float)MAX_RADIUS;   // y-size of each cell
#define N_WIDTH (int) ((MAX_X-MIN_X) / size_x)   // Number of cell-rows
#define N_HEIGHT (int) ((MAX_Y-MIN_Y) / size_y)   // Number of cell-columns

typedef struct {
    float x, y;
    float radius;
} circle;

float randab ( const float a, const float b ) {
    return (float)rand()/(float)RAND_MAX * (b-a) + a;
}

float dist ( const circle a, const circle b ) {
    return hypot( b.x-a.x, b.y-a.y);
}

bool collision( const circle* circles, const int first, const int second ) {
    const float Rsum = circles[first].radius + circles[second].radius;

    if(fabs(circles[first].x - circles[second].x) > Rsum) return false;
    if(fabs(circles[first].y - circles[second].y) > Rsum) return false;

    return ( dist(circles[first], circles[second]) <= Rsum );
}

void init ( const int n, circle* circles ) {
    for(int i=0; i<n; i++) {
        circles[i].x = randab(MIN_X, MAX_X);
        circles[i].y = randab(MIN_Y, MAX_Y);
        circles[i].radius = randab(MIN_RADIUS, MAX_RADIUS);
    }
}

uint64_t serial ( const int n, const circle* circles ) {
    uint64_t coll = 0;
    for(int i=0; i<n; i++) {
        for(int j=i+1; j<n; j++) {
            if(collision(circles, i, j)) {
                coll++;
            }
        }
    }
    return coll;
}

uint64_t parallel ( const int n, const circle* circles ) {
    uint64_t coll = 0;
    const int nth = omp_get_max_threads();
    int *l_coll = (int*) malloc(nth * sizeof(int));
    assert(l_coll);
    for(int i=0; i<nth; i++) l_coll[i] = 0;

    #pragma omp parallel default(none) \
    shared(l_coll, circles)
    {
        const int tid = omp_get_thread_num();
        const int local_start = tid * n / nth;
        const int local_end = (tid+1) * n / nth;

        for(int i=local_start; i<local_end; i++) {
            for(int j=i+1; j<n; j++) {
                if(collision(circles, i, j)) {
                    l_coll[tid]++;
                }
            }
        }
    }

    for(int i=0; i<nth; i++) {
        coll += l_coll[i];
    }
    free(l_coll);

    return coll;
}

int*** init_quadtree ( const int n, const circle* circles ) {

    int*** quadtree = (int***) malloc(N_WIDTH * sizeof(int**));
    assert(quadtree);
    for(int i=0; i<N_WIDTH; i++) {
        quadtree[i] = (int**) malloc(N_HEIGHT * sizeof(int*));
        assert(quadtree[i]);
        for(int j=0; j<N_HEIGHT; j++) {
            quadtree[i][j] = (int*) malloc(MAX_CIRCLES * sizeof(int));
            assert(quadtree[i][j]);
            for(int k=0; k<MAX_CIRCLES; k++) {
                quadtree[i][j][k] = -1;
            }
        }
    }

    // Filling the tree
    for(int i=0; i<n; i++) {
        const int cell_x = min((int)floor(circles[i].x / size_x), N_WIDTH-1);
        const int cell_y = min((int)floor(circles[i].y / size_y), N_HEIGHT-1);

        // Too full check
        assert(quadtree[cell_x][cell_y][MAX_CIRCLES-1] == -1);

        // Inserting circle
        for(int k=0; k<MAX_CIRCLES; k++) {
            if(quadtree[cell_x][cell_y][k] == -1){
                quadtree[cell_x][cell_y][k] = i;
                break;
            }
        }
    }

    return quadtree;
}

void destroy_quadtree ( int*** qtree ) {
    for(int i=0; i<N_WIDTH; i++) {
        for(int j=0; j<N_HEIGHT; j++) {
            free(qtree[i][j]);
        }
        free(qtree[i]);
    }
    free(qtree);
}

uint64_t serial_quadtree ( int*** qtree, const int n, const circle* circles ) {
    uint64_t coll = 0;
    uint64_t neighbors_collisions = 0;

    for(int x=0; x<N_WIDTH; x++) {
        for(int y=0; y<N_HEIGHT; y++) {
            // For each section...

            for(int k=0; k<MAX_CIRCLES; k++) {
                // For each circle in the main section

                if(qtree[x][y][k] == -1) break;

                const int first = qtree[x][y][k];

                // Check against every circle in the main section
                for(int c=k+1; c<MAX_CIRCLES; c++) {
                    if(qtree[x][y][c] == -1) break;

                    const int second = qtree[x][y][c];
                    if(collision(circles, first, second)) {
                        coll++;
                    }
                }

                // And check against every circle in other neighboring sections
                for(int i=x-1; i<=x+1; i++) {
                    if(i<0 || i>=N_WIDTH) continue;
                    for(int j=y-1; j<=y+1; j++) {
                        if(j<0 || j>=N_HEIGHT) continue;
                        if(x==i && y==j) continue;
                        
                        // For each circle...
                        for(int c=0; c<MAX_CIRCLES; c++) {
                            if(qtree[i][j][c] == -1) break;

                            const int second = qtree[i][j][c];
                            
                            if(collision(circles, first, second)) {
                                neighbors_collisions++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Each collision with a neighbor is counted twice
    return coll + neighbors_collisions/2;
}

uint64_t parallel_quadtree ( int*** qtree, const int n, const circle* circles ) {
    uint64_t coll = 0;
    uint64_t neighbors_collisions = 0;

    /*
        Not the best parallelization possible but this will work "well enough"
        to show how fast a parallel quad-tree can be.
    */

    #pragma omp parallel for collapse(2) default(none) \
    shared(qtree, circles) \
    reduction(+:coll,neighbors_collisions)
    for(int x=0; x<N_WIDTH; x++) {
        for(int y=0; y<N_HEIGHT; y++) {
            // For each section...

            for(int k=0; k<MAX_CIRCLES; k++) {
                // For each circle in the main section

                if(qtree[x][y][k] == -1) break;

                const int first = qtree[x][y][k];

                // Check against every circle in the main section
                for(int c=k+1; c<MAX_CIRCLES; c++) {
                    if(qtree[x][y][c] == -1) break;

                    const int second = qtree[x][y][c];
                    if(collision(circles, first, second)) {
                        coll++;
                    }
                }

                // And check against every circle in other neighboring sections
                for(int i=x-1; i<=x+1; i++) {
                    if(i<0 || i>=N_WIDTH) continue;
                    for(int j=y-1; j<=y+1; j++) {
                        if(j<0 || j>=N_HEIGHT) continue;
                        if(x==i && y==j) continue;
                        
                        // For each circle...
                        for(int c=0; c<MAX_CIRCLES; c++) {
                            if(qtree[i][j][c] == -1) break;

                            const int second = qtree[i][j][c];
                            
                            if(collision(circles, first, second)) {
                                neighbors_collisions++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Each collision with a neighbor is counted twice
    return coll + neighbors_collisions/2;
}

void print_ram_usage (unsigned int nbytes) {
    float ram_usage = (float)nbytes;
    if(ram_usage < 1024.0f) {
        fprintf(stderr, "%3.3f bytes", ram_usage);
    }
    else if(ram_usage < 1024.0f*1024.0f) {
        fprintf(stderr, "%3.3f KBytes", ram_usage/1024.0f);
    }
    else {
        fprintf(stderr, "%3.3f MBytes", ram_usage/(1024.0f*1024.0f));
    }
}

int main ( const int argc, const char *argv[] ) {
    srand(time(NULL));

    int ncircles = 10000;
    const int max_ncircles = 10000000;
    circle *circles;
    uint64_t num_collisions;

    if(argc > 2) {
        fprintf(stderr, "Usage: %s [n]\n\n", argv[0]);
        return EXIT_FAILURE;
    }
    else if (argc == 2) {
        ncircles = atoi(argv[1]);
        if(ncircles < 0 || ncircles > max_ncircles) {
            fprintf(stderr, "The number of points must be positive and smaller than %d\n\n", max_ncircles);
            return EXIT_FAILURE;
        }
    }

    fprintf(stderr, "\n\t%d circles\n", ncircles);
    fprintf(stderr, "\t");
    print_ram_usage(ncircles * 3 * sizeof(float));
    fprintf(stderr, " of RAM used\n");
    fprintf(stderr, "\t%d threads used\n\n", omp_get_max_threads());

    circles = (circle*) malloc(ncircles * sizeof(circle));
    assert(circles);

    init(ncircles, circles);

    fprintf(stderr, "Serial:\n");
    double t = omp_get_wtime();
    num_collisions = serial(ncircles, circles);
    t = omp_get_wtime() - t;
    fprintf(stderr, "\t%.3f seconds\n\t%llu collisions\n", t, num_collisions);

    fprintf(stderr, "Parallel:\n");
    t = omp_get_wtime();
    num_collisions = parallel(ncircles, circles);
    t = omp_get_wtime() - t;
    fprintf(stderr, "\t%.3f seconds\n\t%llu collisions\n", t, num_collisions);


    fprintf(stderr, "\nQuad-Tree initialization: ");
    t = omp_get_wtime();
    int*** qtree = init_quadtree(ncircles, circles);
    t = omp_get_wtime() - t;
    fprintf(stderr, "%.3f seconds\n", t);

    fprintf(stderr, "Memory used: ");
    print_ram_usage(N_WIDTH*sizeof(int**) + N_WIDTH*N_HEIGHT*sizeof(int*) + N_WIDTH*N_HEIGHT*MAX_CIRCLES*sizeof(int));
    fprintf(stderr, "\n\n");

    fprintf(stderr, "Serial quad-tree:\n");
    t = omp_get_wtime();
    num_collisions = serial_quadtree(qtree, ncircles, circles);
    t = omp_get_wtime() - t;
    fprintf(stderr, "\t%.3f seconds\n\t%llu collisions\n", t, num_collisions);

    // Parallel quad-tree
    fprintf(stderr, "Parallel quad-tree:\n");
    t = omp_get_wtime();
    num_collisions = parallel_quadtree(qtree, ncircles, circles);
    t = omp_get_wtime() - t;
    fprintf(stderr, "\t%.3f seconds\n\t%llu collisions\n", t, num_collisions);

    destroy_quadtree(qtree);

    free(circles);

    return EXIT_SUCCESS;
}