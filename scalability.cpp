
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
    Lock* lock;
    LockTester(int opt) {
        if(opt==1){
            lock = new Hemlock();
        }
        else{
        
        }
        threadCounts = {1, 2, 4, 8, 16, 32, 64, 128};
    }

    void test(const std::string &outFileName)
    {
        std::ofstream outFile(outFileName);
        outFile.close();

        for (int cnt : threadCounts)
        {
            lock = new Hemlock();
            thrTimes.assign(cnt, 0);

            std::vector<std::thread> threads;
            for (int i = 0; i < cnt; i++)
            {
                threads.emplace_back(&LockTester::threadFunc, this, i);
            }
            for (auto &t : threads)
            {
                t.join();
            }
            
            double avgTime = 0;
            for (int i = 0; i < cnt; i++)
            {
                avgTime += thrTimes[i];
            }
            avgTime /= cnt * 100;

            std::ofstream out(outFileName, std::ios::app);
            out << avgTime << std::endl;
            out.close();

            std::cout << "[" << outFileName << "] threads=" << cnt<< " avgWait=" << avgTime << " us" << std::endl;
        }
    }

private:
    std::vector<long> thrTimes;

    void threadFunc( int threadId)
    {
        std::mt19937 gen(42 + threadId);
        std::exponential_distribution<double> expo1(lambda1);
        std::exponential_distribution<double> expo2(lambda2);
        for (int i = 0; i < 100; i++)
        {
            long reqTime = getCurrTime();
            lock->lock();
            long actTime = getCurrTime();
            long sleepInside = (long)expo1(gen);
            std::this_thread::sleep_for(std::chrono::microseconds(sleepInside));
            lock->unlock();
            thrTimes[threadId] += (actTime - reqTime);
            long sleepOutside = (long)expo2(gen);
            std::this_thread::sleep_for(std::chrono::microseconds(sleepOutside));
        }
    }
};

int main(){
    LockTester tester(1);
    tester.test("scalability_hemlock.txt");
}