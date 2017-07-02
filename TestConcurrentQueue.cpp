#include <cmath>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <sstream>
#include <set>
#include <vector>
#include "ConcurrentQueue.h"


class Obj {
public:
    Obj(int p, uint64_t i) : producer(p), id(i) {
    }
    Obj() : producer(0), id(0) {}
    void operator()() {
        std::ostringstream os;
        os << "Running P: " << producer << " Task: " << id;
        std::cout << os.str() << std::endl << std::flush;
    }
    const int producer;
    const uint64_t id;
};


/**
 * Pool of Long values to test ConcurrentQueue
 */
class Pool {
public:
    void put(uint64_t v) {
        std::lock_guard<std::mutex> lock(mLock);
        elements.insert(v);
    }
    void erase(uint64_t v) {
        std::lock_guard<std::mutex> lock(mLock);
        if (elements.find(v) == elements.end()) {
            std::cerr << "No such element " << v << std::endl;
            std::abort();
        }
        elements.erase(v);
    }
private:
    std::mutex mLock;
    std::set<uint64_t> elements;
};

/**
 * Producer to test ConcurrentQueue
 */
class Producer {
public:
    Producer(int id, Pool& p, uint64_t m) : pool(p), producerId(id), max(m) {
    }
    void operator()(ConcurrentQueue<Obj*>& queue) {
        while(true) {
            auto taskId = counter++;
            if (taskId >= max) {
                break;
            }
            pool.put(taskId);
            auto task = new Obj(producerId, taskId);
            queue.push(task);
        }
    }

private:
    Pool& pool;
    const int producerId;
    const uint64_t max;
    static std::atomic_uint_fast64_t counter;
};

/**
 * Consumer to test ConcurrentQueue
 */
class Consumer {
public:
    Consumer(Pool& p, uint64_t m) : pool(p), max(m) {}
    void operator()(ConcurrentQueue<Obj*>& queue) {
        while (true) {
            if (queue.peek()) {
                Obj* task;
                queue.pop(task);
                pool.erase(task->id);
                uint64_t taskId = task->id;
                (*task)();
                delete task;
                if (taskId == max-1) {
                    break;
                }
            }
        }
    }

private:
    Pool& pool;
    const uint64_t max;
};


std::atomic_uint_fast64_t Producer::counter;

int main(int, char**) {

    constexpr int producerCount = 10;
    constexpr uint64_t limit = 100000;

    ConcurrentQueue<Obj*> queue;

    Pool pool;
    std::thread producerThreads[producerCount];

    std::thread consumerThread([ & ] {
        Consumer consumer(pool, limit);
        consumer(queue);
    });

    for (int i=0; i<producerCount; ++i) {
        producerThreads[i] = std::thread([ &, i ] {
            Producer producer(i, pool, limit);
            producer(queue);
        });
    }

    consumerThread.join();
    for (int i=0; i<producerCount; ++i) {
        producerThreads[i].join();
    }
    return 0;
}