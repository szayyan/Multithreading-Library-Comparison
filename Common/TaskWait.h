#pragma once
#include <mutex>
#include <condition_variable>
#include <iostream>

class TaskWait 
{
    /*  Task waiting barrier - non blocking for tasks but observing object must wait until
        tasks are complete */
public:
    TaskWait(int task_count):
    number_tasks(task_count), 
    completed_tasks(0)
    { }

    void Synchronise()
    {
        /* Thread calling this function must wait until all tasks are complete */
        
        std::unique_lock<std::mutex> lk(m);
        condition.wait(lk,[&] {return completed_tasks == number_tasks;});
        completed_tasks = 0;
    }

    void TaskCompleted()
    {
        {
            std::unique_lock<std::mutex> lk(m);
            completed_tasks++;
        }
        condition.notify_one();
    }

private:
    std::mutex m;
    std::condition_variable condition;
    const int number_tasks;
    int completed_tasks;
};