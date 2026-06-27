#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

#include "../include/lockfree_queue.h"
using namespace std;

const int PRODUCERS = 4;
const int CONSUMERS = 4;
const int ITEMS_PER_PRODUCER = 10000;
const int TOTAL_ITEMS = PRODUCERS * ITEMS_PER_PRODUCER;

LockFreeQueue<int, 8> q;
atomic<int> produced(0);
atomic<int> consumed(0);

void producer(int id){
    int start=id*ITEMS_PER_PRODUCER;
    int end=start+ITEMS_PER_PRODUCER-1;
    for(int number=start;number<=end;number++){
        while(!q.try_enqueue(number));
        produced++;
    }
}

void consumer(vector<int>& result){
    while(true){
        int item;
        if(q.try_dequeue(item)){
            result.push_back(item);
            consumed++;
        }
        else if(produced==TOTAL_ITEMS) break;
    }
}

int main(){
    for(int i=0;i<100;i++){
        produced=0;consumed=0;
        vector<thread> producers;
        vector<thread> consumers;
        vector<vector<int>> consumer_results(CONSUMERS);

        for(int i=0;i<PRODUCERS;i++){
            producers.push_back(thread(producer, i));
        }

        for(int i=0;i<CONSUMERS;i++){
            consumers.push_back(thread(consumer, ref(consumer_results[i])));
        }

        for(auto& t: producers) t.join();
        for(auto& t: consumers) t.join();
        vector<bool> hash(TOTAL_ITEMS, false);
        for(int i=0;i<CONSUMERS;i++){
            for(int j=0;j<consumer_results[i].size();j++){
                int item=consumer_results[i][j];
                if(item<0 || item>=TOTAL_ITEMS || hash[item]){
                    cout<<"Error: Duplicate or invalid item "<<item<<endl;
                    return 1;
                }
                hash[item]=true;
            }
        }
        for(int i=0;i<TOTAL_ITEMS;i++){
            if(!hash[i]){
                cout<<"Missing item: "<<i<<endl;
                return 1;
            }
        }
        cout << "All correctness tests passed!" << endl;
    }
    return 0;
}
