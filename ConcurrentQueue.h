#include <cmath>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <set>
#include <vector>


/**
 * Concurrent Queue to support multiple Producers and Single Consumer
 * @tparam T Type of Element to pass from producers to consumers
 * @tparam SIZE Size of the Queue
 * @tparam MAX_SPIN_ON_BUSY Number of tries before failing to push by a Producer
 */
template<typename T, uint64_t SIZE = 4096, uint64_t MAX_SPIN_ON_BUSY = 40000000>
class ConcurrentQueue {
private:
    static constexpr unsigned Log2(unsigned n, unsigned p = 0) {
        return (n <= 1) ? p : Log2(n / 2, p + 1);
    }

    static constexpr uint64_t closestExponentOf2(int64_t x) {
        return (1UL << ((int64_t) (Log2(SIZE - 1)) + 1));
    }

    static constexpr uint64_t mRingModMask = closestExponentOf2(SIZE) - 1;
    static constexpr uint64_t mSize = closestExponentOf2(SIZE);

    T mMem[mSize];
    std::atomic_int_fast64_t mWriteSlotPtr;
    std::atomic_int_fast64_t mReadPtr;
    std::atomic_int_fast64_t mWritePtr;

public:
    ConcurrentQueue() : mWritePtr(0), mWriteSlotPtr(-1), mReadPtr(0) {}

    /**
     * Pop an element from Queue
     * @param ret Element poped (by reference)
     * @return true if Queue isn't empty and an element is removed
     */
    bool pop(T &ret) {
        // If there is nothing to consume
        if (!peek()) {
            return false;
        }
        int64_t readPtr = mReadPtr;
        // Backup the element
        ret = mMem[readPtr & mRingModMask];
        // Let the Producers know that we are done with this position
        mReadPtr.store(readPtr + 1);
        return true;
    }

    /**
     * Test if Queue has an element to pop
     * @return true if Queue isn't empty
     */
    bool peek() const {
        return (mWritePtr != mReadPtr);
    }

    /**
     * Get the number of elements in the Queue
     * @return Number of elements in Queue
     */
    int64_t getCount() const {
        int64_t readPtr = mReadPtr;
        int64_t writePtr = mWritePtr;
        return writePtr > readPtr ? writePtr - readPtr : readPtr - writePtr;
    }

    /**
     * Push an element into Queue
     * @param pItem Item to Push to Queue
     * @throws runtime_error If Unable to push after MAX_SPIN_ON_BUSY tries
     */
    void push(const T &pItem) {
        for (int i = 0; i < MAX_SPIN_ON_BUSY; ++i) {
            int64_t readPtr = mReadPtr;
            int64_t writePtr = mWritePtr;
            int64_t  size = (writePtr > readPtr ? writePtr - readPtr : readPtr - writePtr);
            if (size >= mSize) {
                continue;
            }
            int64_t writeSlotPtr = writePtr - 1;
            // Get write slot
            if (mWriteSlotPtr.compare_exchange_strong(writeSlotPtr, writePtr)) {
                // This thread got the slot, fill the element
                mMem[writePtr & mRingModMask] = pItem;
                if (!mWritePtr.compare_exchange_strong(writePtr, writePtr + 1)) {
                    std::cerr << "Unable to update mWritePtr to " << writePtr + 1 << std::endl;
                    std::abort();
                }
                return;
            }


        }
        throw std::runtime_error("Concurrent queue full, cannot write to it!");
    }


};
