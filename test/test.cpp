#include "collectives.h"
#include "timer.h"

#include <mpi.h>

#include <stdexcept>
#include <iostream>
#include <vector>

void TestCollectivesCPU(std::vector<size_t> &sizes, std::vector<size_t> &iterations)
{
    // Initialize on CPU (no GPU device ID).
    InitCollectives(NO_DEVICE);

    // Get the MPI size and rank.
    int mpi_size;
    if (MPI_Comm_size(MPI_COMM_WORLD, &mpi_size) != MPI_SUCCESS)
        throw std::runtime_error("MPI_Comm_size failed with an error");

    int mpi_rank;
    if (MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank) != MPI_SUCCESS)
        throw std::runtime_error("MPI_Comm_rank failed with an error");

    timer::Timer timer;
    for (size_t i = 0; i < sizes.size(); i++)
    {
        auto size = sizes[i];
        auto iters = iterations[i];

        float *data = new float[size];
        float seconds = 0.0f;
        for (size_t iter = 0; iter < iters; iter++)
        {
            // Initialize data as a block of ones, which makes it easy to check for correctness.
            for (size_t j = 0; j < size; j++)
            {
                data[j] = 1.0f;
            }

            float *output;
            timer.start();
            RingAllreduce(data, size, &output);
            seconds += timer.seconds();

            // Check that we get the expected result.
            for (size_t j = 0; j < size; j++)
            {
                if (output[j] != (float)mpi_size)
                {
                    std::cerr << "Unexpected result from allreduce: " << data[j] << std::endl;
                    return;
                }
            }
            delete[] output;
        }
        if (mpi_rank == 0)
        {
            std::cout << "Verified allreduce for size "
                      << size
                      << " ("
                      << seconds / iters
                      << " per iteration)" << std::endl;
        }

        delete[] data;
    }
}

// Test program for baidu-allreduce collectives, should be run using `mpirun`.
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./allreduce-test (cpu|gpu)" << std::endl;
        return 1;
    }
    std::string input(argv[1]);

    // Buffer sizes used for tests.
    std::vector<size_t> buffer_sizes = {
        0, 32, 256, 1024, 4096, 16384, 65536, 262144, 1048576, 8388608, 67108864, 536870912};

    // Number of iterations to run for each buffer size.
    std::vector<size_t> iterations = {
        100000, 100000, 100000, 100000,
        1000, 1000, 1000, 1000,
        100, 50, 10, 1};

    // Test on either CPU and GPU.
    if (input == "cpu")
    {
        TestCollectivesCPU(buffer_sizes, iterations);
    }
    else
    {
        std::cerr << "Unknown device type: " << input << std::endl
                  << "Usage: ./allreduce-test (cpu|gpu)" << std::endl;
        return 1;
    }

    // Finalize to avoid any MPI errors on shutdown.
    MPI_Finalize();

    return 0;
}
