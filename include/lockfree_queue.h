#pragma once
#include <atomic>
using namespace std;

template<typename T, int N>
class LockFreeQueue
{
    static_assert((N & (N - 1)) == 0,
                  "Queue size must be a power of two");
private:

    struct Slot
    {
        atomic<int> sequence;
        T value;
    };

    Slot buffer[N];
    alignas(64) atomic<int> enqueue_pos;
    alignas(64) atomic<int> dequeue_pos;

public:

    LockFreeQueue() : enqueue_pos(0), dequeue_pos(0){
        for(int i=0;i<N;i++){
            buffer[i].sequence.store(i, memory_order_relaxed);
        }
    }

    bool try_enqueue(const T& item){
        while(true){
            int pos=enqueue_pos.load(memory_order_relaxed);
            auto &slot=buffer[pos & (N-1)];
            int seq=slot.sequence.load(memory_order_acquire);
            int diff=seq-pos;
            if(diff<0) return false; //queue is full
            else if(diff==0){
                if(enqueue_pos.compare_exchange_weak(pos,pos+1)){
                    slot.value=item;
                    slot.sequence.store(pos+1, memory_order_release);
                    return true;
                }
            }
            else continue;   //the queue state changed, retry
        }
        
    }

    bool try_dequeue(T& item){
        while(true){
            int pos=dequeue_pos.load(memory_order_relaxed);
            auto &slot=buffer[pos & (N-1)];
            int seq=slot.sequence.load(memory_order_acquire);
            int diff=seq-(pos+1);
            if(diff<0) return false; //queue is empty
            else if(diff==0){
                if(dequeue_pos.compare_exchange_weak(pos,pos+1)){
                    item=slot.value;
                    slot.sequence.store(pos+N, memory_order_release);
                    return true;
                }
            }
            else continue;   //the queue state changed, retry
        }
    }
};