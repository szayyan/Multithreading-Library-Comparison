#include <vector>
#include <queue>
#include <memory>
#include "TaskWait.h"
#include "Worker.h"
#include <atomic>

class ThreadPoolAdv
{
    /* More featurefull threadpool with thread specification and randomised work stealing */
public:
    ThreadPoolAdv(int num_workers);
    void AssignWorkToThread(std::function<void()> work, int thread);
    void ExecuteWorkQueue();
    
private:
    // work for each thread
    std::vector<std::queue<std::function<void()>>> thread_work; 
    TaskWait t_wait;
    int worker_steal_count;
    std::vector<std::unique_ptr<Worker>> workers;
};