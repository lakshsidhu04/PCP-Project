#include "hemlock.h"

void Hemlock::lock()
{
    myNode.grant.store(0, std::memory_order_relaxed);
    HemlockQNode *pred = tail.exchange(&myNode, std::memory_order_acq_rel);
    if (pred != nullptr)
    {
        while (pred->grant.load(std::memory_order_acquire) == 0)
        {
            std::this_thread::yield();
        }
    }
}

void Hemlock::unlock()
{
    HemlockQNode *qnode = &myNode;
    if (tail.compare_exchange_strong(qnode, nullptr, std::memory_order_release))
    {
        return;
    }

    HemlockQNode *succ = tail.load(std::memory_order_acquire);
    while (succ == nullptr || succ == qnode)
    {
        succ = tail.load(std::memory_order_acquire);
    }

    succ->grant.store(1, std::memory_order_release);
}