#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include "hemlock.h"

std::mt19937 gen(42);
double lambda1 = 1.0;
double lambda2 = 2.0;
std::vector<int> thrTimes;

long getCurrTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Hemlock lock;
void threadFunc(int threadId){
    std::exponential_distribution<double> expo1(lambda1);
    std::exponential_distribution<double> expo2(lambda2);
    for(int i = 0; i < 100; i++){
        long reqTime = getCurrTime();
        lock.lock();
        long actTime = getCurrTime();
        long sleepInside = expo1(gen);
        std::this_thread::sleep_for(std::chrono::microseconds(int(sleepInside)));
        lock.unlock();
        long exitTime = getCurrTime();
        thrTimes[threadId] += (actTime - reqTime);
        double sleepOutside = expo2(gen);
        std::this_thread::sleep_for(std::chrono::microseconds(int(sleepOutside)));
    }
}

int main(){
    std::vector<int> thrCnts = {1, 2, 4, 8, 16, 32, 64};
    for(int cnt : thrCnts){
        thrTimes.resize(cnt);
        std::vector<std::thread> threads;
        for(int i = 0; i < cnt; i++){
            threads.emplace_back(threadFunc, i);
        }
        for(auto &t : threads){
            t.join();
        }
        double avgTime = 0;
        for(int i = 0; i < cnt; i++){
            avgTime += thrTimes[i];
        }
        avgTime /= cnt*100;
        std::ofstream outFile("scalability_hemlock.txt");
        outFile << avgTime << std::endl;
        outFile.close();
    }

}


