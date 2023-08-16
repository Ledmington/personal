#include <iostream>
#include <random>
#include <chrono>
#include <cstdlib>
#include <hwy/highway.h>

using data_type = double;
using index_type = uint32_t;

std::mt19937 rng{std::random_device{}()};
std::uniform_real_distribution<data_type> dist{0.0, 1.0};

void naive_matmul_2d(data_type **a, data_type **b, data_type **c, const index_type size)
{
    for (index_type i{0}; i < size; i++)
    {
        for (index_type k{0}; k < size; k++)
        {
            const data_type aik = a[i][k];
            for (index_type j{0}; j < size; j++)
            {
                c[i][j] += aik * b[k][j];
            }
        }
    }
}

void naive_matmul_1d(data_type *a, data_type *b, data_type *c, const index_type size)
{
    for (index_type i{0}; i < size; i++)
    {
        for (index_type k{0}; k < size; k++)
        {
            const data_type aik = a[i * size + k];
            for (index_type j{0}; j < size; j++)
            {
                c[i * size + j] += aik * b[k * size + j];
            }
        }
    }
}

HWY_ATTR void vector_matmul_2d(data_type **HWY_RESTRICT a, data_type **HWY_RESTRICT b, data_type **HWY_RESTRICT c, const index_type size)
{
    namespace hn = hwy::HWY_NAMESPACE;
    const hn::ScalableTag<data_type> d;
    using V = hn::Vec<decltype(d)>;

    const index_type lanes = static_cast<index_type>(hn::Lanes(d));

    for (index_type i{0}; i < size; i++)
    {
        for (index_type k{0}; k < size; k++)
        {
            const data_type aik = a[i][k];
            const V vaik = hn::Set(d, aik);
            index_type j{0};
            for (; j + (lanes - 1) < size; j += lanes)
            {
                const V bkj = hn::LoadU(d, b[k] + j);
                V cij = hn::LoadU(d, c[i] + j);
                cij = hn::MulAdd(vaik, bkj, cij);
                hn::StoreU(cij, d, c[i] + j);
            }

            // remainder loop
            for (; j < size; j++)
            {
                c[i][j] += aik * b[k][j];
            }
        }
    }
}

HWY_ATTR void vector_matmul_1d(data_type *HWY_RESTRICT a, data_type *HWY_RESTRICT b, data_type *HWY_RESTRICT c, const index_type size)
{
    namespace hn = hwy::HWY_NAMESPACE;
    const hn::ScalableTag<data_type> d;
    using V = hn::Vec<decltype(d)>;

    const index_type lanes = static_cast<index_type>(hn::Lanes(d));

    for (index_type i{0}; i < size; i++)
    {
        for (index_type k{0}; k < size; k++)
        {
            const data_type aik = a[i * size + k];
            const V vaik = hn::Set(d, aik);
            index_type j{0};
            for (; j + (lanes - 1) < size; j += lanes)
            {
                const V bkj = hn::LoadU(d, b + (k * size + j));
                V cij = hn::LoadU(d, c + (i * size + j));
                cij = hn::MulAdd(vaik, bkj, cij);
                hn::StoreU(cij, d, c + (i * size + j));
            }

            // remainder loop
            for (; j < size; j++)
            {
                c[i * size + j] += aik * b[k * size + j];
            }
        }
    }
}

uint64_t get_timer_resolution()
{
    const uint32_t length = 1000000;
    struct std::chrono::time_point<std::chrono::high_resolution_clock> v[length];

    for (uint32_t i{0}; i < length; i++)
    {
        v[i] = std::chrono::high_resolution_clock::now();
    }

    uint64_t res = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(v[1] - v[0]).count());
    for (uint32_t i{1}; i < length - 1; i++)
    {
        res = std::min(res, static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(v[i + 1] - v[i]).count()));
    }

    return res;
}

void init_2d(data_type **a, data_type **b, data_type **c, const index_type size)
{
    for (index_type i{0}; i < size; i++)
    {
        for (index_type j{0}; j < size; j++)
        {
            a[i][j] = dist(rng);
            b[i][j] = dist(rng);
            c[i][j] = static_cast<data_type>(0);
        }
    }
}

void init_1d(data_type *a, data_type *b, data_type *c, const index_type size)
{
    for (index_type i{0}; i < size * size; i++)
    {
        a[i] = dist(rng);
        b[i] = dist(rng);
        c[i] = static_cast<data_type>(0);
    }
}

void check_2d(data_type **c, const index_type size)
{
    for (index_type i{0}; i < size; i++)
    {
        for (index_type j{0}; j < size; j++)
        {
            if (c[i][j] < 0 || c[i][j] > static_cast<data_type>(size))
            {
                std::cout << "ERROR: invalid result at index " << i << "; " << j << std::endl;
            }
        }
    }
}

void check_1d(data_type *c, const index_type size)
{
    for (index_type i{0}; i < size * size; i++)
    {
        if (c[i] < 0 || c[i] > static_cast<data_type>(size))
        {
            std::cout << "ERROR: invalid result at index " << i << std::endl;
        }
    }
}

double nano_to_seconds(const double ns)
{
    return ns / 1000000000;
}

void run_benchmarks(const uint32_t size, const uint32_t max_iterations)
{
    data_type **a_2d = new data_type *[size];
    data_type **b_2d = new data_type *[size];
    data_type **c_2d = new data_type *[size];
    data_type *a_1d = new data_type[size * size];
    data_type *b_1d = new data_type[size * size];
    data_type *c_1d = new data_type[size * size];

    for (uint32_t i{0}; i < size; i++)
    {
        a_2d[i] = new data_type[size];
        b_2d[i] = new data_type[size];
        c_2d[i] = new data_type[size];
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    double elapsed;

    std::cout << "     | Naive 2D | Vector 2D | Naive 1D | Vector 1D |" << std::endl;

    for (uint32_t it{0}; it < max_iterations; it++)
    {
        printf("| %2d |", it);

        init_2d(a_2d, b_2d, c_2d, size);

        start = std::chrono::high_resolution_clock::now();
        naive_matmul_2d(a_2d, b_2d, c_2d, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        check_2d(c_2d, size);

        printf(" %8.6f |", nano_to_seconds(elapsed));

        init_2d(a_2d, b_2d, c_2d, size);

        start = std::chrono::high_resolution_clock::now();
        vector_matmul_2d(a_2d, b_2d, c_2d, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        check_2d(c_2d, size);

        printf(" %8.6f |", nano_to_seconds(elapsed));

        init_1d(a_1d, b_1d, c_1d, size);

        start = std::chrono::high_resolution_clock::now();
        naive_matmul_1d(a_1d, b_1d, c_1d, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        check_1d(c_1d, size);

        printf(" %8.6f |", nano_to_seconds(elapsed));

        init_1d(a_1d, b_1d, c_1d, size);

        start = std::chrono::high_resolution_clock::now();
        vector_matmul_1d(a_1d, b_1d, c_1d, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        check_1d(c_1d, size);

        printf(" %8.6f |", nano_to_seconds(elapsed));

        std::cout << std::endl;
    }

    for (uint32_t i{0}; i < size; i++)
    {
        delete[] a_2d[i];
        delete[] b_2d[i];
        delete[] c_2d[i];
    }

    delete[] a_2d;
    delete[] b_2d;
    delete[] c_2d;
    delete[] a_1d;
    delete[] b_1d;
    delete[] c_1d;
}

int main(const int argc, const char **argv)
{
    uint32_t max_iterations = 20;
    uint32_t size = 2000;

    // input parsing
    if (argc > 1)
    {
        max_iterations = static_cast<uint32_t>(strtoull(argv[1], NULL, 10));
    }
    if (argc > 2)
    {
        size = static_cast<uint32_t>(strtoull(argv[2], NULL, 10));
    }
    if (argc > 3)
    {
        std::cout << "WARNING: more than 2 arguments provided, ignoring the others." << std::endl;
    }

    // input checking
    if (max_iterations < 1)
    {
        fprintf(stderr, "%d is not a valid number of iterations.\n", max_iterations);
        return -1;
    }
    if (size < 1)
    {
        fprintf(stderr, "%d is not a valid matrix size.\n", size);
        return -1;
    }

    printf("Matrix size               : %d\n", size);
    printf("Iterations                : %d\n", max_iterations);
    printf("Timer resolution          : %ld ns\n", get_timer_resolution());
    printf("Approx. total memory used : %.3f MB\n",
           (double)(6 * (uint64_t)size * (uint64_t)size * sizeof(data_type)) / 1048576.0);
    printf("Vector type               : %ld x %ld bits\n", hwy::HWY_NAMESPACE::Lanes(hwy::HWY_NAMESPACE::ScalableTag<data_type>{}), 8 * sizeof(data_type));
    printf("\n");

    run_benchmarks(size, max_iterations);

    return 0;
}