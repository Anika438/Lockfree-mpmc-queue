#include <mutex>
#include <queue>

using namespace std;
template<typename T>
class MutexQueue {
private:
    mutex mtx;
    queue<T> q;
    int capacity;

public:
    MutexQueue(int capacity=1024) {
        this->capacity = capacity;
    }

    bool try_enqueue(const T& item){
        lock_guard<mutex> lock(mtx);
        if(q.size() < capacity){
            q.push(item);
            return true;
        }
        return false;
    }

    bool try_dequeue(T& item){
        lock_guard<mutex> lock(mtx);
        if(!q.empty()){
            item = q.front();
            q.pop();
            return true;
        }
        return false;
    }
};