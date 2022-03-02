#include "ThreadPoolAdv.h"
#include <algorithm>
#include <iostream>
#include <random>

ThreadPoolAdv::ThreadPoolAdv(int num_workers):
t_wait(num_workers),
// performs well for thread count <= 32
worker_steal_count( std::max(static_cast<int>(std::log2(num_workers)-(num_workers <= 4 ? 0 : 1) ),4) )
{
    

    thread_work.resize(num_workers);
    // construct workers
    for(int i = 0; i < num_workers; i++)
    {
        workers.push_back( std::make_unique<Worker>(&t_wait) );
    }
    // Randomly assign each worker other workers to steal from
    // could extend to prioritise workers spatially close to one another
    std::vector<int> stolen_from;
    std::vector<int> to_steal_from(num_workers);
    std::iota(to_steal_from.begin(),to_steal_from.end(),0);

    std::random_device rd;
    std::mt19937 gen(rd());
    
    // option 1 - true random
    // possiblity of work being wasted because worker could be assigned worker to steal from twice
    
    /*for(int i = 0; i < worker_steal_count; i++)
    {
        std::shuffle(to_steal_from.begin(),to_steal_from.end(),gen);
        for(int j = 0; j < num_workers; j++)
        {
            workers[j]->steal_from.push_back( workers[to_steal_from[j]].get() );
        }
    }*/

    // option 2 - randomly shifted list prevents workers assigned to themselves
    // better performance in real world test
    std::uniform_int_distribution<> dis(1,num_workers-1);
    
    for(int i = 0; i < worker_steal_count; i++)
    {        
        do
        {
            std::rotate(to_steal_from.begin(), to_steal_from.begin() +dis(gen),to_steal_from.end() );
        }
        // prevent stealing from self
        while( to_steal_from[0] == 0 );

        for(int j = 0; j < num_workers; j++)
        {
            workers[j]->steal_from.push_back(workers[to_steal_from[j]].get());
        }
    }
}
 
void ThreadPoolAdv::AssignWorkToThread(std::function<void()> work, int thread) 
{
    thread_work[thread].push( work );
}

void ThreadPoolAdv::ExecuteWorkQueue() 
{
    // apply tasks in chunk
    for(size_t i = 0; i < workers.size(); i++)
    {
        workers[i]->AssignTasks(thread_work[i]);
        std::queue<std::function<void()>> empty;
        std::swap(empty,thread_work[i]);
    }
        // wait untill all threads have executed all tasks before function continues
    t_wait.Synchronise();

}
