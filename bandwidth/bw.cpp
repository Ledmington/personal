#include <iostream>
#include <chrono>
#include <cstdint>
#include <random>
#include <cstdio>
#include <vector>

#define RESET "\033[0m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

using data_type = double;
using index_type = uint32_t;
using result_type = double;

result_type mean(const result_type *values, const size_t length)
{
    result_type s = static_cast<result_type>(0);
    for (size_t i{0}; i < length; i++)
    {
        s += values[i];
    }
    return s / static_cast<result_type>(length);
}

result_type stddev(const result_type *values, const size_t length, const result_type m)
{
    result_type s = static_cast<result_type>(0);
    for (size_t i{0}; i < length; i++)
    {
        s += (values[i] - m) * (values[i] - m);
    }
    return std::sqrt(s / static_cast<result_type>(length));
}

result_type format_bytes(result_type b)
{
    while (b >= 1024)
    {
        b /= 1024;
    }
    return b;
}

std::string biggest_byte_unit_for(result_type b)
{
    result_type bytes = static_cast<result_type>(b);
    if (bytes < 1024)
    {
        return "B";
    }
    bytes /= 1024;
    if (bytes < 1024)
    {
        return "KB";
    }
    bytes /= 1024;
    if (bytes < 1024)
    {
        return "MB";
    }
    bytes /= 1024;
    if (bytes < 1024)
    {
        return "GB";
    }
    return "TB";
}

double nano_to_seconds(const double ns)
{
    return ns / 1000000000;
}

int main(const int argc, const char **argv)
{
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<data_type> dist{static_cast<data_type>(0), static_cast<data_type>(1)};

    uint32_t max_bytes = 1 << 30; // 1 GB by default

    if (argc > 1)
    {
        max_bytes = std::strtoul(argv[1], NULL, 10);
    }
    if (argc > 2)
    {
        std::cout << "I do not need more than one argument. I'll ignore the others." << std::endl;
    }

    const uint32_t max_length = max_bytes / sizeof(data_type);

    std::cout << "Size of a single data element: " << 8 * sizeof(data_type) << " bits" << std::endl;
    std::cout << "Size of an index: " << 8 * sizeof(index_type) << " bits" << std::endl;
    std::cout << "Max elements per array: " << max_length << std::endl;
    std::cout << "I will use 2 arrays" << std::endl
              << std::endl;

    for (uint32_t bytes{sizeof(data_type)}; bytes <= max_bytes; bytes *= 2)
    {
        const uint32_t length = bytes / sizeof(data_type);

        std::vector<data_type> a(length);
        std::vector<data_type> b(length);
        std::vector<result_type> values{};

        double mean_bw = 0.0; // mean value of bandwidth
        double sd_bw = 0.0;   // stddev of the bandwidth
        double hwci_bw = 0.0; // half width confidence interval of the bandwidth
        size_t inside = 0;    // number of values inside the 2*stddev range

        do
        {
            // init vectors
            for (index_type j{0}; j < length; ++j)
            {
                a[j] = dist(rng);
                b[j] = dist(rng);
            }

            const auto start = std::chrono::high_resolution_clock::now();

            // copy a into b
            for (index_type i{0}; i < length; i++)
            {
                b[i] = a[i];
            }

            const auto finish = std::chrono::high_resolution_clock::now();
            const double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());
            /*
                Computing bandwidth as bytes transferred per second.
                The 2 * bytes is there because we are copying values from one
                array to another: 1 copy = 1 read + 1 write
            */
            values.push_back((2 * bytes) / nano_to_seconds(elapsed));

            // check a == b
            for (index_type j{0}; j < length; j++)
            {
                if (a[j] != b[j])
                {
                    std::cout << "ERROR: arrays differ at index " << j << ": " << a[j] << "; " << b[j] << std::endl;
                    break;
                }
            }

            mean_bw = mean(values.data(), values.size());
            sd_bw = stddev(values.data(), values.size(), mean_bw);
            hwci_bw = 2 * sd_bw / std::sqrt(values.size());

            inside = 0;
            // count how many values are inside the 2 stddev range
            for (size_t j{0}; j < values.size(); j++)
            {
                if (values[j] <= (mean_bw + 2 * sd_bw) && values[j] >= (mean_bw - 2 * sd_bw))
                {
                    inside++;
                }
            }
        } while (
            // must run at least one time
            values.size() < 1 ||
            // must have at least one value inside and one outside the 2 stddev range
            values.size() == inside ||
            // the confidence interval must not go on negative values
            hwci_bw >= mean_bw ||
            // at least 0.9545 of the values must be inside 2 standard deviations
            (static_cast<double>(inside) / static_cast<double>(values.size())) < 0.9545);

        printf(CYAN "%7.3f %2s" WHITE " (%12u elements): " GREEN "%7.3f %2s/s" WHITE " +- " YELLOW "%7.3f %2s/s" RESET "\n", format_bytes(bytes), biggest_byte_unit_for(bytes).c_str(), length, format_bytes(mean_bw), biggest_byte_unit_for(mean_bw).c_str(), format_bytes(hwci_bw), biggest_byte_unit_for(hwci_bw).c_str());
    }

    return 0;
}