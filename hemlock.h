#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include "Lock.h"

struct HemlockQNode
{
    std::atomic<int> grant{1};
};

class Hemlock: public Lock
{
public:
    void lock();
    void unlock();

private:
    static thread_local HemlockQNode myNode;
    std::atomic<HemlockQNode *> tail{nullptr};
};