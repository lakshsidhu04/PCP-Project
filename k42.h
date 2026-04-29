#include <atomic>
#include <cstdint>
#include <immintrin.h>
#include "Lock.h"

using namespace std;

class K42Lock: public Lock
{
private:
    struct Node
    {
        atomic<Node *> nextNode{nullptr}; // queue link
        atomic<Node *> waitFlag{nullptr}; // local wait flag
    };

    alignas(64) atomic<Node *> queueNext{nullptr};
    alignas(64) atomic<Node *> queueTail{nullptr};

    static inline void cpuRelax()
    {
        _mm_pause();
    }

public:
    K42Lock()
    {
        queueNext.store(nullptr, memory_order_relaxed);
        queueTail.store(nullptr, memory_order_relaxed);
    }

    K42Lock(const K42Lock &) = delete;
    K42Lock &operator=(const K42Lock &) = delete;

    void lock()
    {
        Node myNode;
        myNode.nextNode.store(nullptr, memory_order_relaxed);

        atomic_thread_fence(memory_order_seq_cst);

        Node *prevNode = queueTail.exchange(&myNode, memory_order_acq_rel);

        if (prevNode != nullptr)
        {
            myNode.waitFlag.store(reinterpret_cast<Node *>(1), memory_order_relaxed);

            atomic_thread_fence(memory_order_seq_cst);

            prevNode->nextNode.store(&myNode, memory_order_release);

            atomic_thread_fence(memory_order_seq_cst);

            while (myNode.waitFlag.load(memory_order_acquire) != nullptr)
            {
                cpuRelax();
            }
        }

        Node *succNode = myNode.nextNode.load(memory_order_acquire);

        if (succNode == nullptr)
        {
            atomic_thread_fence(memory_order_seq_cst);

            queueNext.store(nullptr, memory_order_relaxed);

            Node *expectedNode = &myNode;
            Node *sentinelNode = reinterpret_cast<Node *>(&queueNext);

            if (!queueTail.compare_exchange_strong(
                    expectedNode, sentinelNode,
                    memory_order_release,
                    memory_order_relaxed))
            {
                while ((succNode = myNode.nextNode.load(memory_order_acquire)) == nullptr)
                {
                    cpuRelax();
                }

                queueNext.store(succNode, memory_order_release);
            }
        }
        else
        {
            queueNext.store(succNode, memory_order_release);
        }
    }

    void unlock()
    {
        Node *succNode = queueNext.load(memory_order_acquire);

        atomic_thread_fence(memory_order_seq_cst);

        if (succNode == nullptr)
        {
            Node *expectedNode = reinterpret_cast<Node *>(&queueNext);

            if (queueTail.compare_exchange_strong(
                    expectedNode, nullptr,
                    memory_order_release,
                    memory_order_relaxed))
            {
                return;
            }

            while ((succNode = queueNext.load(memory_order_acquire)) == nullptr)
            {
                cpuRelax();
            }
        }

        succNode->waitFlag.store(nullptr, memory_order_release);
    }

    bool try_lock()
    {
        Node *expectedNode = nullptr;
        Node *sentinelNode = reinterpret_cast<Node *>(&queueNext);
        return queueTail.compare_exchange_strong(
            expectedNode, sentinelNode,
            memory_order_acq_rel,
            memory_order_acquire);
    }
};
