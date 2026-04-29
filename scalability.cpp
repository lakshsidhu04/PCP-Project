
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include "Lock.h"
#include "hemlock.h"
#include "k42.h"

double lambda1 = 1.0;
double lambda2 = 2.0;

long getCurrTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
class LockTester
{
public:
    std::atomic<long> maxWaitTime{0};
    std::vector<int> threadCounts;
    Lock* lock;
    int opt;
    LockTester(int opt) {
        this->opt = opt;
        delete lock;
        if(opt == 1){
            lock = new Hemlock();
        }
        else if(opt == 2){
            lock = new K42Lock();
        }
        threadCounts = {1, 2, 4, 8, 16, 32, 64};
    }

    void test(const std::string &outFileName,const std::string &maxTimeFileName)
    {
        std::ofstream outFile(outFileName);
        outFile.close();
        
        for (int cnt : threadCounts)
        {
            delete lock;
            if(opt == 1){
                lock = new Hemlock();
            }
            else if(opt == 2){
                lock = new K42Lock();
            }
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
            double variance = 0;
            for (int i = 0; i < cnt; i++)
            {
                double diff = thrTimes[i] - avgTime;
                variance += diff * diff;
            }
            variance /= cnt;
            std::ofstream out(outFileName, std::ios::app);
            out << avgTime << std::endl;
            std::ofstream maxTimeFile(maxTimeFileName, std::ios::app);
            maxTimeFile << maxWaitTime.load() << " " << variance << std::endl;
            out.close();
            maxTimeFile.close();

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
            if (thrTimes[threadId] > maxWaitTime.load())
            {
                maxWaitTime.store(thrTimes[threadId]);
            }
            long sleepOutside = (long)expo2(gen);
            std::this_thread::sleep_for(std::chrono::microseconds(sleepOutside));
        }
    }
};

int main(){
    LockTester tester1(1);
    tester1.test("scalability_hemlock.txt", "max_wait_hemlock.txt");
    LockTester tester2(2);
    tester2.test("scalability_k42.txt", "max_wait_k42.txt");
    return 0;
}
