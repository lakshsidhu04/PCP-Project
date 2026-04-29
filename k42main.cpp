#include "k42.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

K42Lock globalLock;
long long sharedCount = 0;

// The total number of lock acquisitions we want to test per thread
const int opsPerThread = 1000000;

void doWork()
{
    for (int i = 0; i < opsPerThread; ++i)
    {
        globalLock.lock();

        // critical section
        sharedCount++;
        // Simulate a small operation, like traversing a short linked list
        for (volatile int j = 0; j < 50; ++j)
        {
        }
        //

        globalLock.unlock();
    }
}

int main(int argc, char *argv[])
{
    // Default 4 threads if not specified on command line
    int numThreads = 4;
    if (argc > 1)
    {
        numThreads = stoi(argv[1]);
    }

    cout << "Testing K42 Lock with " << numThreads << " threads...\n";
    vector<thread> threads;

    // Start the timer
    auto startTime = chrono::high_resolution_clock::now();

    // Spawn the threads
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(doWork);
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    // Stop the timer
    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = endTime - startTime;

    // Calculate Throughput (Operations per Second)
    long long totalOps = static_cast<long long>(numThreads) * opsPerThread;
    double throughput = totalOps / elapsed.count();

    // Verify correctness
    cout << "Expected Counter: " << totalOps << "\n";
    cout << "Actual Counter:   " << sharedCount << "\n";

    if (sharedCount == totalOps)
    {
        cout << "[SUCCESS] Lock is mathematically correct\n";
    }
    else
    {
        cout << "[FAILURE] Race condition detected\n";
    }

    cout << "Time Elapsed:     " << elapsed.count() << " seconds\n";
    cout << "Throughput:       " << throughput / 1000000.0 << " Million Ops/Sec\n";

    return 0;
}
