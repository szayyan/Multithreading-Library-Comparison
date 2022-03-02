#include "BFS.h"
#include <stack>
#include <omp.h>

BFS::BFS( std::shared_ptr<Graph> _graph, int _source, int _max_bag_size ):
graph(_graph),
source(_source),
max_bag_size(_max_bag_size),
visited( graph->Size(), 0)
{}

void BFS::SearchOMP(int number_threads) 
{
    /* Search the given graph using OMP */

    omp_set_num_threads(number_threads);
    max_depth = number_threads;// std::ceil(std::log2(number_threads));
    
    // allocate memory for starting bag and begin search
    VertexBag* start_bag = new VertexBag();
    start_bag->PushVertex(source);
    visited[source] = 1;

    // begin OpenMP parallel region 
    #pragma omp parallel
    {
        // a single thread executes this code block
        #pragma omp single
        {
            while( !start_bag->Empty() )
            {
                VertexBag* bag_to_fill = new VertexBag();
                ProccessLevelOMP( start_bag, bag_to_fill ,0);
                std::swap( start_bag, bag_to_fill );
                delete bag_to_fill;
            }
        }
    }
    
    delete start_bag;
}

void BFS::SearchSTDThread(int number_threads) 
{
    //number_threads--;
    max_depth = std::log2(number_threads);
    std::cout << max_depth << std::endl;
    // allocate memory for starting bag and begin search
    VertexBag* start_bag = new VertexBag();
    start_bag->PushVertex(source);
    visited[source] = 1;

    // begin thread loop - threads are now ready to receive tasks
    thread_pool.SpinThreads(number_threads-1);

    while(!start_bag->Empty())
    {
        VertexBag* bag_to_fill = new VertexBag();
        ProccessLevelSTDThread( start_bag, bag_to_fill,0 );
        std::swap( start_bag, bag_to_fill );
        delete bag_to_fill;
    }
    delete start_bag;
}

int BFS::VisitCount() 
{
    /* counts the number of visited threads */
    int visited_count = 0;
    for(size_t i = 0; i < visited.size(); i++)
    {
        if( visited[i] )
            visited_count++;
    }
    return visited_count;
}

void BFS::ProccessLevelOMP(VertexBag*& in_bag, VertexBag*& out_bag, int depth ) 
{
    /* Searches the vertices of all neighbours in in_bag and places them in out_bag using OMP */

    // if bag is too large - break it down and search smaller bag in parallel
    if( in_bag->GetSize() > max_bag_size )
    {
        // split up in bag equally
        VertexBag* in_bag_2 = in_bag->Split();
        VertexBag* out_bag_2 = new VertexBag();

        // spawn an omp task to recursively search the split bag in parallel
        #pragma omp task    default(none) \
                            shared(visited,graph,max_depth) \
                            firstprivate(in_bag,out_bag,depth) \
                            final(depth > max_depth) mergeable
        ProccessLevelOMP(in_bag, out_bag, depth+1 );

        //#pragma omp task    default(none) \
                            shared(visited,graph,max_depth) \
                            firstprivate(in_bag_2,out_bag_2,depth)
        ProccessLevelOMP(in_bag_2, out_bag_2, depth+1 );
        
        // wait for both tasks to finish executing before continuing
        #pragma omp taskwait

        // bag 2 assumes control of vertices proccessed from the other threads bag
        out_bag->Merge( out_bag_2 );

        // clean up memory
        delete out_bag_2;
        delete in_bag_2;
    }
    else
    {
        // iterate through bag 1's pennant 
        // nodes will contain all vertices in the bags pennant
        std::stack<Node*> nodes;
        for(int i = 0; i < in_bag->largest + 1; i++)
        {
            if( in_bag->pennants[i] != nullptr )
            {
                nodes.push( in_bag->pennants[i]->root );
                while (!nodes.empty())
                {
                    Node* node = nodes.top();
                    nodes.pop();
                    if( node->left != nullptr )
                        nodes.push(node->left);
                    if( node->right != nullptr )
                        nodes.push(node->right);

                    // iterate through all neighbours of each vertex and add them to bag 2
                    std::vector<int>& neighbours = graph->adjacency_list[node->vertex];
                    for(size_t j = 0; j < neighbours.size(); j++)
                    {
                        // potential for benign data race here - can be eliminated by atomic CAS
                        // only visit vertex if is has not already been visited
                        if( !visited[neighbours[j]] ) 
                        {
                            visited[neighbours[j]] = 1;
                            out_bag->PushVertex( neighbours[j] );
                        }
                    } 
                }
            }
        }
    }
}

void BFS::ProccessLevelSTDThread(VertexBag*& in_bag, VertexBag*& out_bag, int depth ) 
{
    /* Searches the vertices of all neighbours in in_bag and places them in out_bag using OMP */
   
    // if bag is too large - split it up and execute search smaller bags in parallel
    if( depth < max_depth && in_bag->GetSize() > max_bag_size )
    {
        // split up in bag equally
        VertexBag* in_bag_2 = in_bag->Split();
        VertexBag* out_bag_2 = new VertexBag();

        // need to synchronise 1 tasks
        TaskWait tw(1);

        // spawn tasks in and queue them in thread pool
        Request task1 { std::bind(&BFS::ProccessLevelSTDThread,this,in_bag_2,out_bag_2,depth+1), &tw };
        //Request task2 { std::bind(&BFS::ProccessLevelSTDThread,this,in_bag,out_bag,depth+1 ), &tw };

        thread_pool.EnqueueTask(task1);
        //thread_pool.EnqueueTask(task2);
        
        ProccessLevelSTDThread(in_bag,out_bag,depth+1 );
        tw.Synchronise();

        // out bag assumes control of vertices proccessed from the other threads bag
        out_bag->Merge( out_bag_2 );

        // clean up memory
        delete out_bag_2;
        delete in_bag_2;
    }
    else
    {
        // iterate through bag 1's pennant 
        // nodes will contain all vertices in the bags pennant
        std::stack<Node*> nodes;
        for(int i = 0; i < in_bag->largest + 1; i++)
        {
            if( in_bag->pennants[i] != nullptr )
            {
                nodes.push( in_bag->pennants[i]->root );
                while (!nodes.empty())
                {
                    Node* node = nodes.top();
                    nodes.pop();
                    if( node->left != nullptr )
                        nodes.push(node->left);
                    if( node->right != nullptr )
                        nodes.push(node->right);

                    // iterate through all neighbours of each vertex and add them to bag 2
                    std::vector<int>& neighbours = graph->adjacency_list[node->vertex];
                    for(size_t j = 0; j < neighbours.size(); j++)
                    {   
                        // potential for benign data race here - can be eliminated by atomic CAS
                        // only visit vertex if is has not already been visited  
                        if( !visited[neighbours[j]] )
                        {
                            visited[neighbours[j]] = 1;
                            out_bag->PushVertex( neighbours[j] );
                        }
                    } 
                }
            }
        }
    }
}