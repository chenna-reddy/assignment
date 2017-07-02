#include <cmath>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>

template<typename T, uint64_t SIZE = 4096, uint64_t MAX_SPIN_ON_BUSY = 40000000>
class ConcurrentQueue {
private:
    static constexpr unsigned Log2(unsigned n, unsigned p = 0) {
        return (n <= 1) ? p : Log2(n / 2, p + 1);
    }

    static constexpr uint64_t closestExponentOf2(uint64_t x) {
        return (1UL << ((uint64_t) (Log2(SIZE - 1)) + 1));
    }

    static constexpr uint64_t mRingModMask = closestExponentOf2(SIZE) - 1;
    static constexpr uint64_t mSize = closestExponentOf2(SIZE);

    static const T mEmpty;

    T mMem[mSize];
    std::mutex mLock;
    uint64_t mReadPtr = 0;
    uint64_t mWritePtr = 0;

public:
    T pop() {
        if (!peek()) {
            return mEmpty;
        }

        std::lock_guard<std::mutex> lock(mLock);

        if (!peek()) {
            return mEmpty;
        }

        T ret = mMem[mReadPtr & mRingModMask];

        mReadPtr++;
        return ret;
    }

    bool peek() const {
        return (mWritePtr != mReadPtr);
    }

    uint64_t getCount() const {
        return mWritePtr > mReadPtr ? mWritePtr - mReadPtr : mReadPtr - mWritePtr;
    }

    bool busyWaitForPush() {
        uint64_t start = 0;
        while (getCount() == mSize) {
            if (start++ > MAX_SPIN_ON_BUSY) {
                return false;
            }
        }
        return true;
    }

    void push(const T& pItem) {
        while (true) {
            if (!busyWaitForPush()) {
                throw std::runtime_error("Concurrent queue full cannot write to it!");
            }
            std::lock_guard<std::mutex> lock(mLock);
            if (getCount() < mSize) {
                mMem[mWritePtr & mRingModMask] = pItem;
                mWritePtr++;
                break;
            }
        }
    }

    void push(T&& pItem) {
        if (!busyWaitForPush()) {
            throw std::runtime_error("Concurrent queue full cannot write to it!");
        }

        std::lock_guard<std::mutex> lock(mLock);
        mMem[mWritePtr & mRingModMask] = std::move(pItem);
        mWritePtr++;
    }
};

template<typename T, uint64_t SIZE, uint64_t MAX_SPIN_ON_BUSY>
const T ConcurrentQueue<T, SIZE, MAX_SPIN_ON_BUSY>::mEmpty = T{ };

class Obj {
public:
    Obj(int p, int i) : producer(p), id(i) {
        std::ostringstream os;
        os << "Constructing P: " << producer << " Task: " << id;
        std::cout << os.str() << std::endl << std::flush;
    }
    Obj() : producer(0), id(0) {}
    void operator()() {
        std::ostringstream os;
        os << "Running P: " << producer << " Task: " << id;
        std::cout << os.str() << std::endl << std::flush;
    }
private:
    const int producer;
    const int id;
};

int main(int, char**) {

    ConcurrentQueue<Obj*> queue;


    std::thread consumer([ & ] {
        while (true) {
            if (queue.peek()) {
                auto task = queue.pop();
                (*task)();
                delete task;
            }
        }
    });
    std::atomic_long counter;

    std::thread producer1([ & ] {
        while (true) {
            auto taskId = counter++;
            auto newTask = new Obj(1, taskId);
            queue.push(newTask);
        }
    });


    std::thread producer2([ & ] {
        while (true) {
            auto taskId = counter++;
            auto newTask = new Obj(2, taskId);
            queue.push(newTask);
        }
    });


    consumer.join();
    producer1.join();
    producer2.join();
    return 0;
}
