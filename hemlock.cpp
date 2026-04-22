#include "hemlock.h"
#include <iostream>

thread_local std::vector<HemlockQNode> Hemlock::nodes(2);
thread_local HemlockQNode *Hemlock::myNode = &Hemlock::nodes[0];
thread_local bool Hemlock::flag = true;

void Hemlock::lock()
{
    myNode = flag ? &nodes[0] : &nodes[1];
    flag = !flag;

    myNode->grant.store(1, std::memory_order_relaxed);

    HemlockQNode *pred = tail.exchange(myNode, std::memory_order_acq_rel);

    if (pred != nullptr)
    {
        while (pred->grant.load(std::memory_order_acquire) == 1)
        {
            std::this_thread::yield();
        }
    }
}

void Hemlock::unlock()
{
    std::cout << "Thread " << std::this_thread::get_id() << " released lock\n";
    myNode->grant.store(0, std::memory_order_release);
}