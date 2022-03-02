#pragma once
#include <mutex>
#include <condition_variable>
#include <iostream>

class Barrier 
{
    /* Class designed to synchronise a given number of threads */
public:
    Barrier(int number_threads): 
    level(0),
    thread_count(number_threads), 
    active_threads(number_threads)
    { }

    // Each thread will wait until all other threads have
    // called function and only then will synchronise

    // safe from spurious wakeups and reusable
    void Synchronise()
    {
        std::unique_lock<std::mutex> lk(m);
        active_threads--;
        if( active_threads == 0 )
        {
            // all threads have reached the barrier, notify other threads they can continue
            active_threads = thread_count;
            level++;
            condition.notify_all();            
        }
        else
        {
            int cur_level = level;
            condition.wait(lk, [&] {return cur_level != level;});
        }
    }

private:
    std::mutex m;
    std::condition_variable condition;
    const int thread_count;
    int active_threads;
    int level;
};