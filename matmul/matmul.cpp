#include <iostream>
#include <random>
#include <chrono>
#include <hwy/highway.h>

void matmul(double **a, double **b, double **c, const uint32_t size)
{
    for (uint32_t i{0}; i < size; i++)
    {
        for (uint32_t k{0}; k < size; k++)
        {
            for (uint32_t j{0}; j < size; j++)
            {
                c[i][j] += (a[i][k] * b[k][j]);
            }
        }
    }
}

HWY_ATTR void vector_matmul(double **HWY_RESTRICT a, double **HWY_RESTRICT b, double **HWY_RESTRICT c, const uint32_t size)
{
    namespace hn = hwy::HWY_NAMESPACE;
    const hn::ScalableTag<double> d;
    using V = hn::Vec<decltype(d)>;

    const uint32_t lanes = static_cast<uint32_t>(hn::Lanes(d));

    for (uint32_t i{0}; i < size; i++)
    {
        for (uint32_t k{0}; k < size; k++)
        {
            const V a_i_k = hn::Set(d, a[i][k]);
            for (uint32_t j{0}; j + (lanes - 1) < size; j += lanes)
            {
                const V b_k_j = hn::LoadU(d, b[k] + j);
                V c_i_j = hn::LoadU(d, c[i] + j);
                c_i_j = hn::MulAdd(a_i_k, b_k_j, c_i_j);
                hn::StoreU(c_i_j, d, c[i] + j);
            }
        }
    }
}

double nano_to_seconds(const double ns)
{
    return ns / 1000000000;
}

int main(void)
{
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist{0.0, 1.0};

    const uint32_t size = 1024;

    double **a = new double *[size];
    double **b = new double *[size];
    double **c = new double *[size];

    for (uint32_t i{0}; i < size; i++)
    {
        a[i] = new double[size];
        b[i] = new double[size];
        c[i] = new double[size];
    }

    const uint32_t max_iterations = 100;
    auto start = std::chrono::high_resolution_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    double elapsed;

    for (uint32_t it{0}; it < max_iterations; it++)
    {
        // matrix initialization
        for (uint32_t i{0}; i < size; i++)
        {
            for (uint32_t j{0}; j < size; j++)
            {
                a[i][j] = dist(rng);
                b[i][j] = dist(rng);
                c[i][j] = 0.0;
            }
        }

        start = std::chrono::high_resolution_clock::now();
        matmul(a, b, c, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        // check result
        for (uint32_t i{0}; i < size; i++)
        {
            for (uint32_t j{0}; j < size; j++)
            {
                if (c[i][j] < 0 || c[i][j] > static_cast<double>(size))
                {
                    std::cout << "ERROR: invalid result at index " << i << "; " << j << std::endl;
                }
            }
        }

        std::cout << nano_to_seconds(elapsed) << " seconds";

        // matrix initialization
        for (uint32_t i{0}; i < size; i++)
        {
            for (uint32_t j{0}; j < size; j++)
            {
                a[i][j] = dist(rng);
                b[i][j] = dist(rng);
                c[i][j] = 0.0;
            }
        }

        start = std::chrono::high_resolution_clock::now();
        vector_matmul(a, b, c, size);
        finish = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());

        // check result
        for (uint32_t i{0}; i < size; i++)
        {
            for (uint32_t j{0}; j < size; j++)
            {
                if (c[i][j] < 0 || c[i][j] > static_cast<double>(size))
                {
                    std::cout << "ERROR: invalid result at index " << i << "; " << j << std::endl;
                }
            }
        }

        std::cout << "\t" << nano_to_seconds(elapsed) << " seconds" << std::endl;
    }

    for (uint32_t i{0}; i < size; i++)
    {
        delete[] a[i];
        delete[] b[i];
        delete[] c[i];
    }

    delete[] a;
    delete[] b;
    delete[] c;

    return 0;
}