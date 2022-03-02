#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include "TaskWait.h"

struct Request
{
    std::function<void()> task;
    TaskWait* task_wait;
};

class ThreadPool
{
    /*  ThreadPool for executing tasks concurrently with thread reusage */
public:
    void SpinThreads(int number_threads);
    void EnqueueTask(Request request);
    ~ThreadPool();
private:
    std::vector<std::thread> threads;
    std::queue<Request> requests;
    std::condition_variable q_condition;
    std::mutex q_mutex;
    bool finished = false;
};