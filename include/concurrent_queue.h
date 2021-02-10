#ifndef _CONCURRENT_QUEUE_H_
#define _CONCURRENT_QUEUE_H_

#include <mutex>
#include <memory>
#include <condition_variable>

namespace producer_consumer {

template<typename T, uint64_t SIZE = 4096>
class ConcurrentQueue {
private:
    static constexpr unsigned Log2(unsigned n, unsigned p = 0) {
        return (n <= 1) ? p : Log2(n / 2, p + 1);
    }

    static constexpr uint64_t closestExponentOf2(uint64_t x) {
        return (1UL << ((uint64_t) (Log2(SIZE - 1)) + 1));
    }

    static constexpr uint64_t mRingModMask = closestExponentOf2(SIZE) - 1; // due to 0-based circular array
    static constexpr uint64_t mSize = closestExponentOf2(SIZE); // size of queue

    std::shared_ptr<T> mMem[mSize];
    std::mutex mLock;
    std::condition_variable mCV;
    uint64_t mReadPos = 0;
    uint64_t mWritePos = 0;

public:
    const std::shared_ptr<T> pop() {
        std::unique_lock<std::mutex> lock(mLock);

        if (isEmpty()) { // check if queue is empty
            if(mCV.wait_until(lock, std::chrono::system_clock::now()+std::chrono::seconds(1), [this]{ return isEmpty(); })) {
                return nullptr; // timeout happened, no wake up signal received
            }
        }

        auto ret = std::move(mMem[mReadPos & mRingModMask]);

        mReadPos++;

        mCV.notify_one(); // notify producer thread
        return ret;
    }

    bool isEmpty() const {
        return (mWritePos == mReadPos);
    }

    bool isFull() const {
        return (getCount() == mSize);
    }

    uint64_t getCount() const {
        return mWritePos > mReadPos ? mWritePos - mReadPos : mReadPos - mWritePos;
    }

    void push(std::shared_ptr<T>&& pItem) {

        std::unique_lock<std::mutex> lock(mLock);

        if(isFull()) { // should wait until consumer thread wakes it up!
            if(mCV.wait_until(lock, std::chrono::system_clock::now()+std::chrono::seconds(1), [this]{ return isFull(); })) {
                return; // timeout happened, no wake up signal received
            }
        }

        mMem[mWritePos & mRingModMask] = std::move(pItem);
        mWritePos++;

        mCV.notify_one(); // notify consumer thread
    }

};

}

#endif // _CONCURRENT_QUEUE_H_
