#include <atomic>
#include <thread>

class alignas(64) HemlockQNode
{
public:
    std::atomic<int> grant{0};
};

class Hemlock
{
    std::atomic<HemlockQNode *> tail{nullptr};
    thread_local static HemlockQNode myNode;

public:
    void lock();
    void unlock();
};

thread_local HemlockQNode Hemlock::myNode;