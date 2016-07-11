#include "cqueue.h"

#include <cassert>

cqueue::cqueue(size_t capacity)
    : mCapacity(capacity)
    , mBegin(0)
    , mEnd(0)
    , mSize(0)
{
    mStorage.resize(mCapacity); // not reserve. is intentional
    assert(mCapacity == mStorage.size() && "unexpected std::vector implementation");
}

bool cqueue::Enqueue(int val)
{
    std::lock_guard<std::mutex> _(mLock);
    
    if (mSize == mCapacity)
        return false; // early return, capacity exceeded;

    mSize++;
    mStorage[mEnd++] = val;

    if (mEnd >= mCapacity)
        mEnd = 0;
    return true;
}

bool cqueue::Dequeue(int& res)
{
    std::lock_guard<std::mutex> _(mLock);
    
    if (0 == mSize)
        return false; // early return, empty;

    mSize--;
    res = mStorage[mBegin++];

    if (mBegin >= mCapacity)
        mBegin = 0;
    return true;
}
