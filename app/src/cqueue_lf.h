#ifndef __CQUEUE_LF_H__
#define __CQUEUE_LF_H__

#include <atomic>
#include <cassert>
#include <mutex>
#include <vector>

class cqueue_lf
{
public:

    cqueue_lf(uint32_t capacity)
        : mCapacity(capacity)
        , mBegin(0)
        , mEnd(0)
    {
        assert(capacity > 1);
        mBuf.resize(capacity);
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
        //return (val + 1) % mCapacity;
        return val + 1;
    }

    const uint32_t mCapacity;
    std::atomic<uint32_t> mBegin;
    std::atomic<uint32_t> mEnd;
    std::mutex mWriteLock;
    std::vector<int> mBuf;
};

#endif
