#ifndef __CQUEUE_LF_H__
#define __CQUEUE_LF_H__

#include <atomic>
#include <cassert>
#include <mutex>
#include <vector>

struct Atomics
{
    std::atomic<uint32_t> begin;
    std::atomic<uint32_t> end;
};

struct Whole
{
    uint32_t begin;
    uint32_t end;
};

union CombinedState
{
    Atomics atomics;
    std::atomic<Whole> whole;
};

class cqueue_lf
{
public:

    cqueue_lf(uint32_t capacity)
        : mCapacity(capacity)
    {
        assert(capacity > 0);
        mBuf.resize(capacity);

        mState.atomics.begin = 0;
        mState.atomics.end = 0;
        assert(true == mState.atomics.begin.is_lock_free() && "!!!");
        assert(true == mState.atomics.end.is_lock_free() && "!!!");
        assert(true == mState.whole.is_lock_free() && "!!!");
    }

    bool Enqueue(int val);
    bool Dequeue(int& val);

private:
    inline uint32_t cur(uint32_t val) const
    {
        return val % mCapacity;
    }
    inline uint32_t next(uint32_t val) const
    {
        return val + 1;
    }

    const uint32_t mCapacity;
    CombinedState mState;
    std::mutex mWriteLock;
    std::vector<int> mBuf;
};

#endif
