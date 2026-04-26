#pragma once
#include <thread>
#include <cassert>
#include <atomic>
#include <cstdint>

#ifdef __linux__
#include <sched.h>
static inline int get_numa_node()
{
    unsigned cpu = 0, node = 0;
    #ifdef SYS_getcpu
        ::syscall(SYS_getcpu, &cpu, &node, nullptr);
    #endif
        return static_cast<int>(node);
}
#else
    static inline int get_numa_node() { return 0; }
#endif


struct alignas(64) CNA_QNode
{
    std::atomic<int> state{0};
    std::atomic<CNA_QNode *> next{nullptr};
    std::atomic<CNA_QNode *> nextSec{nullptr};
    int NUMA_Node{-1};
};

class CNALock{
public:
    void lock();
    void unlock();
private:
    alignas(64) std::atomic<CNA_QNode *> tail{nullptr};
    alignas(64) std::atomic<CNA_QNode *> head{nullptr};
    alignas(64) std::atomic<CNA_QNode *> secHead{nullptr};
    alignas(64) std::atomic<CNA_QNode *> secTail{nullptr};
    CNA_QNode *findSameNUMAThread(CNA_QNode *node, CNA_QNode*& secTail, CNA_QNode *succ);
};