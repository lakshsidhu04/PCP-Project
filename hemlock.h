#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include "Lock.h"

struct HemlockQNode
{
    std::atomic<int> grant{1};
};

class Hemlock : public Lock
{
public:
    void lock()
    {
        myNode.grant.store(0, std::memory_order_release);
        HemlockQNode *pred = tail.exchange(&myNode, std::memory_order_acq_rel);
        if (pred != nullptr)
        {
            while (pred->grant.load(std::memory_order_acquire) != 1)
            {
                std::this_thread::yield();
            }
            pred->grant.store(0, std::memory_order_release);
        }
    }
    void unlock()
    {
        HemlockQNode *expec = &myNode;
        if (tail.compare_exchange_strong(expec, nullptr, std::memory_order_acq_rel))
        {
            return;
        }
        myNode.grant.store(1, std::memory_order_release);
        while (myNode.grant.load(std::memory_order_acquire) != 0)
        {
            std::this_thread::yield();
        }
    }

private:
    static thread_local HemlockQNode myNode;
    std::atomic<HemlockQNode *> tail{nullptr};
};

thread_local HemlockQNode Hemlock::myNode;