#include "hemlock.h"
#include <iostream>

thread_local HemlockQNode Hemlock::myNode;

void Hemlock::lock()
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

void Hemlock::unlock()
{
    HemlockQNode* expec = &myNode;
    if (tail.compare_exchange_strong(expec, nullptr, std::memory_order_acq_rel))
    {
        return;
    }
    myNode.grant.store(1, std::memory_order_release);
    while(myNode.grant.load(std::memory_order_acquire) != 0)
    {
        std::this_thread::yield();
    }
}