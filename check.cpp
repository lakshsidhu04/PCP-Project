#include <thread>
#include <vector>
#include <cassert>
#include <iostream>
#include "hemlock.h"

Hemlock lk;
long long counter = 0;
const int THREADS = 8;
const int ITERS   = 1000;

void worker() {
    for (int i = 0; i < ITERS; i++) {
        lk.lock();
        std::cout << "Thread " << std::this_thread::get_id() << " acquired lock\n";
        ++counter;
        lk.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; i++)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    assert(counter == (long long)THREADS * ITERS);
    std::cout << "PASS: counter = " << counter << "\n";
}