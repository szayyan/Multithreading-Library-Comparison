#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <cmath>
#include <iostream>
#include <string>
#include <atomic>
#include "Barrier.h"

template<typename T,typename S>
class OrderedMapAndReduceLayer
{
    /* layer wise synchronised map and reduce
       designed for associative but not neccassarily commutative reduction operands */

public:
    OrderedMapAndReduceLayer( int num_threads, std::vector<std::vector<T>>& raw_data );
    S ApplyMapReduce( std::function<S(S,S)> merge_func );
    void MapReduce(int index, int thread_count);
    S FinaliseMerge();
private:
    std::vector<std::thread> threads;
    Barrier barrier;
    std::vector<std::vector<T>>& raw_data;
    std::vector<S> proccessed_data;
    std::function<S(S,S)> merge_func;
};

template<typename T, typename S>
OrderedMapAndReduceLayer<T, S>::OrderedMapAndReduceLayer( int num_threads, std::vector<std::vector<T>>& _raw_data ):
threads(num_threads-1),
proccessed_data(num_threads),
raw_data(_raw_data),
barrier(num_threads)
{}

template<typename T, typename S>
S OrderedMapAndReduceLayer<T, S>::ApplyMapReduce( std::function<S(S,S)> _reduce_func ) 
{
    /* Apply map and reduction to all elements in the list */

    merge_func = _reduce_func;

    for(size_t i = 0; i < threads.size(); i++)
        // begin reduction
        threads[i] = std::thread(&OrderedMapAndReduceLayer::MapReduce,this,i,threads.size()+1);
}

template<typename T, typename S>
void OrderedMapAndReduceLayer<T, S>::MapReduce(int index, const int thread_count)
{
    // apply map
    proccessed_data[index] = S(raw_data[index]);

    // reduce
    int max_depth = 1;
    while (max_depth < thread_count)
        max_depth <<= 1;

    bool finished = false;
    int merge_width = 1;
    while (merge_width < max_depth)
    {
        barrier.Synchronise();

        if( (index/merge_width)%2 == 0 && !finished)
        {
            if( index + merge_width < proccessed_data.size() )
                proccessed_data[index] = merge_func( proccessed_data[index], proccessed_data[index+merge_width]);
        }
        else
        {
            finished = true;
        }
        
        merge_width <<= 1;
    }
}

template<typename T, typename S>
S OrderedMapAndReduceLayer<T, S>::FinaliseMerge() 
{
    for(size_t i = 0; i < threads.size();i++)
        // terminate on completion
        threads[i].join();

    return proccessed_data[0];
}