#pragma once
#include <atomic>
#include <thread>
#include <vector>

struct HemlockQNode
{
    std::atomic<int> grant{1};
};

class Hemlock
{
public:
    void lock();
    void unlock();

private:
    static thread_local std::vector<HemlockQNode> nodes;
    static thread_local HemlockQNode *myNode;
    static thread_local bool flag;

    std::atomic<HemlockQNode *> tail{nullptr};
};