#include "BFSVisualised.h"
#include <omp.h>
#include <iostream>
#include <stack>

BFSVisualised::BFSVisualised(   std::shared_ptr<Graph> _graph, int min_par_depth, ParallelData _data,
                                std::vector<glm::vec3>& _node_colours, const std::vector<glm::vec3>& _thread_colours,
                                std::vector<glm::vec3>& _line_colours, std::vector<DirectedEdge>& edges_drawn ):
                                graph(_graph), 
                                granularity(min_par_depth),
                                node_colours(_node_colours),
                                thread_colours(_thread_colours),
                                line_colours(_line_colours),
                                visited(_graph->Size(),0),
                                edges(edges_drawn),
                                thread_ids(_data.num_threads),
                                is_omp(_data.thread_or_omp)                    
{ 
    /* see BFS.cpp for more detailed explanation of class */

    omp_set_num_threads(_data.num_threads);
    start_bag = new VertexBag();
    // begin search from pseudo random vertex - note there is no need for this to be truly random
    // unlike the other random cases in the project so rand() is used instead of <random>
    srand(time(0));
    int _source = rand() % graph->Size();
    start_bag->PushVertex(_source);
    visited[_source] = 1;

    if( is_omp == 2 )
    {
        thread_pool.SpinThreads(_data.num_threads-1);
        max_depth = std::log2(_data.num_threads);
    }
}

bool BFSVisualised::Iterate() 
{
    if( is_omp == 1 )
        return IterateOMP();
    return IterateSTD();
}

bool BFSVisualised::IterateOMP()
{
    // search is complete
    if( start_bag->Empty() )
        return true;
    
    // otherwise continue search
    #pragma omp parallel
    {
        #pragma omp single
        {
            VertexBag* bag_to_fill = new VertexBag();
            ProccessLevelOMP( start_bag, bag_to_fill, 0);
            std::swap( start_bag, bag_to_fill );
            delete bag_to_fill;
        }
    }
    return false;  
}

bool BFSVisualised::IterateSTD() 
{
    if( start_bag->Empty() )
        return true;
    
    VertexBag* bag_to_fill = new VertexBag();
    ProccessLevelSTD( start_bag, bag_to_fill,0 );
    std::swap( start_bag, bag_to_fill );
    delete bag_to_fill;

    return false;
}

void BFSVisualised::ProccessLevelOMP( VertexBag*& in_bag, VertexBag*& out_bag, int depth )
{
    if( in_bag->GetSize() > granularity )
    {
        VertexBag* in_bag_2 = in_bag->Split();
        VertexBag* out_bag_2 = new VertexBag();

        #pragma omp task    default(none) \
                            shared(visited,graph,max_depth) \
                            firstprivate(in_bag,out_bag,depth) \
                            final(depth > max_depth) mergeable
        ProccessLevelOMP(in_bag, out_bag, depth+1 );

        ProccessLevelOMP(in_bag_2, out_bag_2, depth+1);
        
        #pragma omp taskwait

        out_bag->Merge( out_bag_2 );

        delete out_bag_2;
        delete in_bag_2;
    }
    else
    {
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
                    std::vector<int>& neighbours = graph->adjacency_list[node->vertex];
                    for(size_t j = 0; j < neighbours.size(); j++)
                    {
                        DirectedEdge cur_edge{ node->vertex,neighbours[j]};
                        // find the index of the edge we are currently searching
                        int edge_index = std::find(edges.begin(),edges.end(),cur_edge) - edges.begin();
                        // set the edge colour to the appropiate thread
                        glm::vec3 tr_colour = thread_colours[omp_get_thread_num()];
                        line_colours[ 2*edge_index ] = tr_colour;
                        line_colours[ 2*edge_index + 1] = tr_colour;
                        
                        if( !visited[neighbours[j]] )
                        {
                            node_colours[neighbours[j]] = tr_colour;
                            visited[neighbours[j]] = 1;
                            out_bag->PushVertex( neighbours[j] );
                        }
                    } 
                }
            }
        }
    }
}

BFSVisualised::~BFSVisualised()
{
    delete start_bag;
}

void BFSVisualised::ProccessLevelSTD(VertexBag*& in_bag, VertexBag*& out_bag,int depth) 
{
    if( depth < max_depth && in_bag->GetSize() > 0 )
    {
        VertexBag* in_bag_2 = in_bag->Split();
        VertexBag* out_bag_2 = new VertexBag();

        TaskWait tw(1);

        Request task1 { std::bind(&BFSVisualised::ProccessLevelSTD,this,in_bag_2,out_bag_2,depth+1), &tw };
        //Request task2 { std::bind(&BFSVisualised::ProccessLevelSTD,this,in_bag,out_bag,depth+1 ), &tw };

        thread_pool.EnqueueTask(task1);
        //thread_pool.EnqueueTask(task2);
        ProccessLevelSTD(in_bag,out_bag, depth+1);
        tw.Synchronise();

        out_bag->Merge( out_bag_2 );

        delete out_bag_2;
        delete in_bag_2;
    }
    else
    {
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
                        DirectedEdge cur_edge{ node->vertex,neighbours[j]};
                        int edge_index = std::find(edges.begin(),edges.end(),cur_edge) - edges.begin();
                        
                        int ind_colour;
                        {
                            // acquire lock and check if thread has a index yet, if not give it one
                            std::unique_lock<std::mutex> lk(id_mutex);
                            std::thread::id t_id = std::this_thread::get_id();
                            auto iter = std::find(thread_ids.begin(),thread_ids.end(),std::this_thread::get_id());
                            
                            if( iter == thread_ids.end() )
                            {
                                thread_ids.push_back( t_id );
                                ind_colour = thread_ids.size() - 1;
                            }
                            else
                            {
                                ind_colour = iter - thread_ids.begin();
                            }
                        }

                        glm::vec3 tr_colour = thread_colours[ind_colour];
                        line_colours[ 2*edge_index ] = tr_colour;
                        line_colours[ 2*edge_index + 1] = tr_colour;
                        
                        if( !visited[neighbours[j]] )
                        {
                            node_colours[neighbours[j]] = tr_colour;
                            visited[neighbours[j]] = 1;
                            out_bag->PushVertex( neighbours[j] );
                        }
                    } 
                }
            }
        }
    }
}