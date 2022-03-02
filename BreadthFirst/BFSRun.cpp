#include "BFSRun.h"
#include <iostream>

BFSRun::BFSRun(ParallelData _data ):
data(_data),
graph( std::make_shared<Graph>(Graph::GenerateRandomGraph(data.n,6,0.325)) ),
bfs(graph,0,2500)
{
    std::cout << "Initialising Breadth First Search Benchmark" << std::endl <<
                "Vertex Count: " << data.n << std::endl <<
                "Thread Count: " << data.num_threads << std::endl;

}

void BFSRun::Run() 
{
    if( data.thread_or_omp == 1 )
        bfs.SearchOMP(data.num_threads);
    else
        bfs.SearchSTDThread(data.num_threads);
}

void BFSRun::OutputStats() 
{
    std::cout << "Visited Node count: " << bfs.VisitCount() << std::endl;
}