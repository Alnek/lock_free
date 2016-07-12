#include "cqueue_lf.h"

bool cqueue_lf::Enqueue(int val)
{
    std::lock_guard<std::mutex> lk(mWriteLock);

    for (;;)
    {
        Whole state = mState.whole.load();
        uint32_t begin = state.begin;
        uint32_t end = state.end;

        static uint32_t REDUCE = mCapacity * 100000; // this value must be big because of A-B-A issue;
        if (begin >= REDUCE)
        {
            Whole newState = state;
            newState.begin -= REDUCE;
            newState.end -= REDUCE;
            assert(cur(newState.begin) == cur(state.begin));
            assert(cur(newState.end) == cur(state.end));
            mState.whole.compare_exchange_strong(state, newState);
            continue;
        }

        if (end - begin >= mCapacity)
            return false; // no capacity

        mBuf[cur(end)] = val;
        mState.atomics.end.fetch_add(1);
        return true;
    }
}

bool cqueue_lf::Dequeue(int& val)
{
    for (;;)
    {
        Whole state = mState.whole.load();
        uint32_t begin = state.begin;
        uint32_t end = state.end;

        if (begin == end)
        {
            return false; // empty
        }

        val = mBuf[cur(begin)];

        if (true == mState.atomics.begin.compare_exchange_strong(begin, next(begin)))
        {
            return true;
        }
    }
}
