#include "ThreadPool.h"

void ThreadPool::SpinThreads(int number_threads)
{
    /* Start loop for main threads */

    for(int i = 0; i < number_threads; i++)
    {
        threads.emplace_back(
        [&]
        {
            // iterate until destruction 
            while( 1 )
            {
                Request future_request;
                {
                    // acquires lock and waits until task has been received 
                    std::unique_lock<std::mutex> lk(q_mutex);
                    q_condition.wait(lk, [&] { return !requests.empty() || finished;} );
                    if( finished )
                        return;
                    future_request = requests.front();
                    requests.pop();                   
                }
                future_request.task();

                // notifies waiting object that task is complete
                future_request.task_wait->TaskCompleted();
            }
        }
        );
    }
}

void ThreadPool::EnqueueTask(Request request) 
{
    /* Adds a new task to the queue to be processed */
    {
        std::unique_lock<std::mutex> lk(q_mutex);
        requests.emplace( request );
    }
    q_condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(q_mutex);
        finished = true;
    }
    q_condition.notify_all();
    // terminates all workers when pool is destroyed 
    for(auto &worker: threads)
        worker.join();
}