#include "CNA.h"

void CNALock::lock()
{
    
}

void CNALock::unlock()
{
}

CNA_QNode* CNALock::findSameNUMAThread(CNA_QNode* curr, CNA_QNode*& secTail, CNA_QNode* succ)
{
    int myNUMA = curr->NUMA_Node;
    CNA_QNode* node = succ;
    CNA_QNode* localCopy = secTail;
    while(node!=nullptr){
        CNA_QNode *nextNext = node->next.load(std::memory_order_acquire);
        if(node->NUMA_Node == myNUMA){
            curr->next.store(nextNext, std::memory_order_release);
            if(nextNext == nullptr){
                CNA_QNode* expected = node;
                if(secTail->next.compare_exchange_strong(expected, curr, std::memory_order_acq_rel)){
                    while (node->next.load(std::memory_order_acquire) == nullptr)
                        std::this_thread::yield();
                    curr->next.store(node->next.load(std::memory_order_acquire),std::memory_order_release);
                }
            }
            secTail = localCopy;
            return node;
        }
        else{
            node->nextSec.store(nullptr, std::memory_order_release); 
            if(localCopy == nullptr){
                secHead.store(node, std::memory_order_release);
            }
            else{
                localCopy->nextSec.store(node, std::memory_order_release);
            }
            localCopy = node;
            curr = node;
            node = nextNext;
        }
        secTail = localCopy;
        return nullptr;
    }
}



