
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include "Lock.h"
#include "hemlock.h"

double lambda1 = 1.0;
double lambda2 = 2.0;

long getCurrTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
class LockTester
{
public:
    std::vector<int> threadCounts;
    Lock *lock;
    LockTester(int opt)
    {
        if (opt == 1)
        {
            lock = new Hemlock();
        }
        else
        {
        }
        threadCounts = {1, 2, 4, 8, 16, 32, 64, 128};
    }

    void test(const std::string &outFileName)
    {
        std::ofstream outFile(outFileName);
        outFile.close();

        for (int cnt : threadCounts)
        {
            delete lock;
            lock = new Hemlock();
            std::vector<std::thread> threads;
            long startTime = getCurrTime();
            for (int i = 0; i < cnt; i++)
            {
                threads.emplace_back(&LockTester::threadFunc, this, i);
            }
            for (auto &t : threads)
            {
                t.join();
            }
            long endTime = getCurrTime();
            
            double timeSec = (endTime - startTime) / 1e6;
            double throughput = (cnt * 100) / timeSec;
            std::ofstream out(outFileName, std::ios::app);
            out << throughput << std::endl;
            out.close();
        }
    }

private:
    void threadFunc(int threadId)
    {
        std::mt19937 gen(42 + threadId);
        std::exponential_distribution<double> expo1(lambda1);
        std::exponential_distribution<double> expo2(lambda2);
        for (int i = 0; i < 100; i++)
        {
            lock->lock();
            long sleepInside = (long)expo1(gen);
            std::this_thread::sleep_for(std::chrono::microseconds(sleepInside));
            lock->unlock();
            long sleepOutside = (long)expo2(gen);
            std::this_thread::sleep_for(std::chrono::microseconds(sleepOutside));
        }
    }
};

int main()
{
    LockTester tester(1);
    tester.test("throughput_hemlock.txt");
}