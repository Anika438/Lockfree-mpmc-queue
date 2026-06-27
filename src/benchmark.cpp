#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>


#include "../include/mutex_queue.h"
#include "../include/lockfree_queue.h"

using namespace std;

const int THREADS = 4;
const int ITEMS_PER_THREAD = 100000;
const int TOTAL_ITEMS = THREADS * ITEMS_PER_THREAD;

template<typename Queue>
double benchmark(const string& name)
{
    Queue q;
    atomic<int> produced(0);
    atomic<int> consumed(0);
    auto producer=[&](int id){
        int start=id*ITEMS_PER_THREAD;
        int end=start+ITEMS_PER_THREAD-1;
        for(int number=start;number<=end;number++){
            while(!q.try_enqueue(number));
            produced++;
        }
    };
    auto consumer=[&](){
        while(true){
            int item;
            if(q.try_dequeue(item)){
                consumed++;
            }
            else if(produced.load()==TOTAL_ITEMS) break;
        }
    };

    auto start = chrono::steady_clock::now();
    vector<thread> producers;
    vector<thread> consumers;        
    for(int i=0;i<THREADS;i++)
        producers.emplace_back(producer,i);

    for(int i=0;i<THREADS;i++)
        consumers.emplace_back(consumer);

    for(auto &t: producers)
        t.join();

    for(auto &t: consumers)
        t.join();

    auto end = chrono::steady_clock::now();
    auto duration =chrono::duration_cast<chrono::milliseconds>(end-start);
    double seconds = duration.count() / 1000.0;
    double throughput =
        (2.0 * TOTAL_ITEMS) / seconds;
    ofstream file("benchmark_results.csv", ios::app);

    file << name << ","
        << duration.count() << ","
        << throughput << "\n";
    file.close();
    return throughput;
}
    

int main()
{
    ofstream file("benchmark_results.csv");
    file << "Queue,Time(ms),Throughput\n";
    file.close();
    double mutex_sum = 0;
    double lockfree_sum = 0;

    cout << "===== Mutex Queue =====\n";
    for(int i = 0; i < 5; i++)
    {
        double temp= benchmark<MutexQueue<int>>("Mutex Queue");
        cout << "Run " << i+1 << ": " << temp << " ops/sec" << endl;
        mutex_sum += temp;
    }

    cout << "Average Throughput: "<< mutex_sum / 5 << " ops/sec" <<endl;

    cout << "===== LockFree Queue =====\n";
    for(int i = 0; i < 5; i++)
    {
        double temp = benchmark<LockFreeQueue<int,1024>>("LockFree Queue");
        cout << "Run " << i+1 << ": " << temp << " ops/sec" << endl;
        lockfree_sum += temp;
    }

    cout << "Average Throughput: " << lockfree_sum / 5 << " ops/sec" << endl;
}