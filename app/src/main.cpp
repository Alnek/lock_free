#include "cqueue.h"
#include "cqueue_lf.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
#include <windows.h>

//================================================================================================================================

typedef cqueue_lf testqueue;
//typedef cqueue testqueue;

const int ITEMS = 100000;

struct TLS
{
    std::vector<int> buffer;
};

std::atomic<bool> startFlag(false);
std::atomic<bool> doneFlag(false);
std::atomic<uint32_t> prodCnt(0);
std::atomic<uint32_t> consCnt(0);

void produce(testqueue* gueue, TLS* tls)
{
    tls->buffer.reserve(ITEMS);

    while (false == startFlag.load());

    while (prodCnt.load() < ITEMS)
    {
        const int val = rand();
        bool res = gueue->Enqueue(val);

        if (res)
        {
            tls->buffer.push_back(val);

            uint32_t cur = prodCnt.load();
            while (false == prodCnt.compare_exchange_strong(cur, cur + 1));
        }
    }
}

void consume(testqueue* gueue, TLS* tls)
{
    tls->buffer.reserve(10 * ITEMS);

    while (false == startFlag.load());

    while (false == doneFlag.load() || prodCnt.load() != consCnt.load())
    {
        int val = 0;
        bool res = gueue->Dequeue(val);

        if (res)
        {
            tls->buffer.push_back(val);

            uint32_t cur = consCnt.load();
            while (false == consCnt.compare_exchange_strong(cur, cur + 1));
        }
    }
}

void test_one_by_one()
{
    testqueue queue(64);
    for (auto i = 0; i != 1000; ++i)
    {
        const int write = rand();
        assert(true == queue.Enqueue(write) && "failed to Enqueue");
        int read = 0;
        assert(true == queue.Dequeue(read) && "failed to Dequeue");
        assert(write == read && "unexpected value");
    }
}

void test_read_empty()
{
    testqueue queue(64);
    int read = 0;
    assert(false == queue.Dequeue(read) && "Dequeue() must return false");
}

void test_overflow()
{
    testqueue queue(64);
    int first = rand();

    assert(true == queue.Enqueue(first) && "failed to Enqueue");
    for (auto i = 0; i != 1000; ++i)
    {
        const int write = rand();
        queue.Enqueue(write);
    }

    int read = -1;
    assert(true == queue.Dequeue(read) && "failed to Dequeue");
    assert(first == read && "unexpected value");
}

void test_multithreaded()
{
    testqueue queue(64);

    const int consumersNum = 1 + rand() % 8;
    const int producersNum = 1 + rand() % 8;
    std::cout << "consumers: " << consumersNum << "\n";
    std::cout << "producers: " << producersNum << "\n";

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // thread local storages to gather statistics/metrics
    std::vector<TLS> consumersTls;
    consumersTls.resize(consumersNum);
    std::vector<TLS> producersTls;
    producersTls.resize(producersNum);

    // spawn consumers/producers
    for (auto i = 0; i != consumersNum; ++i) consumers.emplace_back(std::thread(consume, &queue, &consumersTls[i]));
    for (auto i = 0; i != producersNum; ++i) producers.emplace_back(std::thread(produce, &queue, &producersTls[i]));
    // start
    const uint64_t start = GetTickCount64();
    startFlag.store(true);

    // wait until producers finish    
    for (auto& thread : producers) thread.join();
    doneFlag.store(true);
    for (auto& thread : consumers) thread.join();

    const uint64_t elapsed = GetTickCount64() - start;
    std::cout << "finished in: " << elapsed << " ms\n";

    // code below is an attempt to validate queued and dequeued numbers
    std::vector<int> enqueued;
    std::vector<int> dequeued;
    enqueued.reserve(producersNum * ITEMS);
    dequeued.reserve(producersNum * ITEMS);

    // extract data from TLSs
    for (const TLS& tls : producersTls)
    {
        dequeued.insert(dequeued.end(), tls.buffer.begin(), tls.buffer.end());
    }
    for (const TLS& tls : consumersTls)
    {
        enqueued.insert(enqueued.end(), tls.buffer.begin(), tls.buffer.end());
    }

    // order is undefined - compare sorted - looks like the only way to compare consistency at least of produced/consumed numbers 
    std::sort(enqueued.begin(), enqueued.end());
    std::sort(dequeued.begin(), dequeued.end());
    assert(enqueued == dequeued);
    std::cout << "enq=" << enqueued.size() << "\n";
    std::cout << "deq=" << enqueued.size() << "\n";
}

int main()
{
    // randomize
    srand(static_cast<uint32_t>(time(nullptr)));

    // tests
    test_one_by_one();
    test_read_empty();
    test_overflow();
    test_multithreaded();

    std::cout << "done\n";

    return 0;
}
