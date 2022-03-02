#include "Worker.h"
#include <iostream>

Worker::Worker(TaskWait* t):
task_wait(t),
tasks_in(false),
finished(false),
pause( ATOMIC_VAR_INIT(false) )
{
    thread = std::thread([&] 
    {
        while(!finished)
        {
            // wait untill task chunk is received then begin exection
            {
                std::unique_lock<std::mutex> lk(q_mutex);
                alert_condition.wait(lk,[&] { return tasks_in || finished; });
            }
            if( finished )
                break;
            tasks_in = false;
            ExecuteTasks();
        }
    });
}

void Worker::ExecuteTasks() 
{   
    /* Executes task queue one by one */
    while(1)
    {
        // acquire mutex for each task in the queue and begin executing
        std::function<void()> task_to_execute;
        {
            std::unique_lock<std::mutex> lk(q_mutex);
            if( task_queue.empty() || finished )
                break;
            task_to_execute = task_queue.front();
            task_queue.pop();
        }
        task_to_execute();
    }
    
    // all tasks completed - try to steal from donor
    
    std::function<void()> stolen_task;
    for(size_t i = 0; i < steal_from.size(); i++ )
       while( steal_from[i]->Steal(stolen_task) )
           stolen_task();

    // both donors and current worker have completed all of their tasks
    // hence synchronise
    task_wait->TaskCompleted(); 
}

void Worker::AssignTasks(std::queue<std::function<void()>> assigned_tasks) 
{
    /*  Assign task queue. When using must wait until all tasks have completed before assigning new work else
        some tasks may not be executed
        thread safe.*/

    {
        std::unique_lock<std::mutex> lk(q_mutex);
        tasks_in = true;
        task_queue = assigned_tasks;
    }
    alert_condition.notify_one();    
}

bool Worker::Steal(std::function<void()>& stolen_task)
{
    /* Give tasks to another worker to execute
       thread safe */

    std::unique_lock<std::mutex> lk(q_mutex);
    if( task_queue.size() == 0 || finished)
        return false;

    stolen_task = task_queue.front();
    task_queue.pop();
    return true;
}

Worker::~Worker() 
{
    //pause.store( true, std::memory_order_relaxed );
    {
        std::unique_lock<std::mutex> lk(q_mutex);
        finished = true;
    }
    alert_condition.notify_one();
    thread.join();
}