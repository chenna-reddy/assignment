#include <cmath>
#include <functional>
#include <iostream>
#include <thread>
#include <sstream>
#include <set>
#include <vector>
#include "ConcurrentQueue.h"


/**
 * Object to pass around between producers and consumers
 */
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
 * Test Configuration
 */
class TestConfig {
public:
    /**
     * Test Configuration
     * @param threadCount No of Producers
     * @param testSize No of elements to test
     * @param sleepStrategy Sleep strategy
     */
    TestConfig(uint threadCount, uint64_t testSize, int sleepStrategy) : threads(threadCount), max(testSize), counter(0), sleepStart(sleepStrategy) {
    }
    void sleep() {
        if (sleepStart == 0) {
            std::chrono::nanoseconds ns(std::rand()%100);
            std::this_thread::sleep_for(ns);
        } else {
            std::chrono::nanoseconds ns(sleepStart);
            std::this_thread::sleep_for(ns);
        }
    }
    Pool pool;
    const uint threads;
    const uint64_t max;
    const int sleepStart;
    std::atomic_uint_fast64_t counter;
};


std::ostream& operator<<(std::ostream& os, const TestConfig& tc) {
    os << "{ threads: " << tc.threads << ", max: " << tc.max << ", sleep: " << tc.sleepStart << "}";
    return os;
}

/**
 * Producer to test ConcurrentQueue
 */
class Producer {
public:
    Producer(int id, TestConfig& testConfig) : tConfig(testConfig), producerId(id) {
    }

    void operator()(ConcurrentQueue<Obj *> &queue) {
        while (true) {
            auto taskId = tConfig.counter++;
            if (taskId >= tConfig.max) {
                break;
            }
            tConfig.sleep();
            tConfig.pool.put(taskId);
            auto task = new Obj(producerId, taskId);
            queue.push(task);
        }
    }

private:
    const int producerId;
    TestConfig& tConfig;
};

/**
 * Consumer to test ConcurrentQueue
 */
class Consumer {
public:
    Consumer(TestConfig& testConfig) : tConfig(testConfig) {}

    void operator()(ConcurrentQueue<Obj *> &queue) {
        long count = 0;
        while (true) {
            tConfig.sleep();
            if (queue.peek()) {
                Obj *task;
                if (!queue.pop(task)) {
                    std::cerr << "pop must not fail" << std::endl;
                    std::abort();
                }
                uint64_t taskId = task->id;
                // std::cout << "Got element " << taskId << std::endl;
                count++;
                tConfig.pool.erase(taskId);
                (*task)();
                delete task;
                if (count == tConfig.max) {
                    break;
                }
            }
        }
    }

private:
    TestConfig& tConfig;
};


void test(TestConfig& testConfig) {
    std::cout << "Testing with " << testConfig << std::endl;
    ConcurrentQueue<Obj *> queue;

    Pool pool;
    std::thread producerThreads[testConfig.threads];

    for (int i = 0; i < testConfig.threads; ++i) {
        producerThreads[i] = std::thread([&, i] {
            Producer producer(i, testConfig);
            producer(queue);
        });
    }


    std::thread consumerThread([&] {
        Consumer consumer(testConfig);
        consumer(queue);
    });

    consumerThread.join();

    for (int i = 0; i < testConfig.threads; ++i) {
        producerThreads[i].join();
    }

}


int main(int, char **) {
    std::cout << "Number of threads: " << std::thread::hardware_concurrency() << std::endl;
    for (int i=std::thread::hardware_concurrency()*2+1; i>0; i-=3) {
        for (int s=0; s<=5; s+=5) {
            TestConfig t1(i, 10000, s);
            test(t1);
        }
    }
    return 0;
}
