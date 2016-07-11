#include "cqueue_lf.h"

bool cqueue_lf::Enqueue(int val)
{
    std::lock_guard<std::mutex> lk(mWriteLock);

    for (;;)
    {
        uint32_t end = mEnd.load();
        uint32_t begin = mBegin.load();

        /*
        static const uint32_t reduce = 1000 * mCapacity;
        if (begin > reduce)
        {
            while (false == mEnd.compare_exchange_strong(end, end - reduce));
            while (false == mBegin.compare_exchange_strong(begin, begin - reduce));
            continue;
        }*/

        if (end - begin >= mCapacity)
            return false; // no capacity

        mBuf[cur(end)] = val;

        if (true == mEnd.compare_exchange_strong(end, next(end)))
        {
            return true;
        }
    }
}

bool cqueue_lf::Dequeue(int& val)
{
    for (;;)
    {
        uint32_t begin = mBegin.load();
        uint32_t end = mEnd.load();

        if (begin == end)
            return false; // empty

        val = mBuf[cur(begin)];

        if (true == mBegin.compare_exchange_strong(begin, next(begin)))
        {
            return true;
        }
    }
}
