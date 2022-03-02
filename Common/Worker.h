#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include "TaskWait.h"

class Worker
{
    /* Worker class - executes give task chunks */
public:
    Worker(TaskWait* t);
    void ExecuteTasks();
    void AssignTasks( std::queue<std::function<void()>> assigned_tasks );
    bool Steal(std::function<void()>& stolen_task);
    void ThreadLoop();
    ~Worker();
    std::vector<Worker*> steal_from;
    TaskWait* task_wait;
    std::thread thread;
    std::mutex alert_mutex;
    std::condition_variable  alert_condition;
    bool tasks_in;
private:
    int work_steal_size;
    std::mutex q_mutex;
    std::queue<std::function<void()>> task_queue;
    std::atomic<bool> pause;
    bool finished;
};
