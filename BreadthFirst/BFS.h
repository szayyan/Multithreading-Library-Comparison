#include "Graph3D.h"
#include <memory>
#include <map>
#include <atomic>
#include "VertexBag.h"
#include "ThreadPool.h"

class BFS
{
public:
    BFS( std::shared_ptr<Graph> _graph, int source, int max_bag_size );
    void SearchOMP(int number_threads);
    void SearchSTDThread(int number_threads);
    int VisitCount();
private:
    // graph to be searched
    std::shared_ptr<Graph> graph;

    // the largest a bag can be
    int max_bag_size;

    // the starting vertex
    int source;

    // stores whether a given vertex has been visited
    std::vector<short> visited;

    // thread pool used for task execution 
    ThreadPool thread_pool;

    // the maximum recursive depth of the tasks
    int max_depth;

    // counts the number of searched edges
    std::atomic<int> edges_searched = ATOMIC_VAR_INIT(0);

    void ProccessLevelOMP( VertexBag*& in_bag, VertexBag*& out_bag, int depth );
    void ProccessLevelSTDThread( VertexBag*& in_bag, VertexBag*& out_bag, int depth );
};

