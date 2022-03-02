#include "Runnable.h"
#include "ParallelData.h"
#include "Graph.h"
#include <memory>
#include "BFS.h"

class BFSRun: public Runnable
{
    /* executes breath first search benchmark */
public:
    BFSRun(ParallelData _data );
    void Run();
    void OutputStats();
private:
    ParallelData data;
    std::shared_ptr<Graph> graph;
    BFS bfs;
};
