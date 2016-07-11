#ifndef __CQUEUE_H__
#define __CQUEUE_H__

#include <mutex>
#include <vector>

/*
    The best choice would be to use lock-free container here, but I'm just started to read about lock-free data structures - so it is not an option to implement one at the moment;
    

    This implementation uses std::mutex to protect shared data;
    Since both (Enqueue and Deque) actually writes data (because of auto-pop front in Dequeue) we need just a single mutex here;
*/
class cqueue
{
public:
    cqueue(size_t capacity);

    // val - value to store
    // returns true on success, false on exceeded capacity
    bool Enqueue(int val);

    // res - value in front, valid if return is true;
    // returns false if queue is empty;
    // pops front
    bool Dequeue(int& res);

    //TODO size(), empty() etc

private:
    cqueue(const cqueue&);
    cqueue(cqueue&&);
    cqueue& operator=(const cqueue&);

    const size_t mCapacity;
    size_t mBegin;
    size_t mEnd;
    size_t mSize;

    std::mutex mLock;
    std::vector<int> mStorage;
};

#endif
