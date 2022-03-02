#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <cmath>
#include <iostream>
#include <string>

/* Depreciated in favour of OrderedMapAndReduceLayer.h */

template<typename T,typename S>
class OrderedMapAndReduce
{
    // pairwise synchronised map and reduce
    // designed for associative but not neccassarily commutative reduction operands
    // works for 2^n threads only

public:
    OrderedMapAndReduce( int num_threads, std::vector<std::vector<T>>& raw_data );
    S ApplyMapReduce( std::function<S(S,S)> merge_func );
private:
    void MapReduce(int index, int thread_count);
    void MapReduceSync(int index, int thread_count);
    std::vector<std::thread> threads;
    std::vector<std::mutex> mutexes;
    std::vector<std::condition_variable> merge_check;
    std::vector<short> merge_active;
    std::vector<std::vector<T>>& raw_data;
    std::vector<S> proccessed_data;

    std::function<S(S,S)> merge_func;
};

template<typename T, typename S>
OrderedMapAndReduce<T, S>::OrderedMapAndReduce( int num_threads, std::vector<std::vector<T>>& _raw_data ):
threads(num_threads),
merge_check(num_threads-1),
merge_active(num_threads-1,0),
proccessed_data(num_threads),
mutexes(num_threads-1),
raw_data(_raw_data)
{}

template<typename T, typename S>
S OrderedMapAndReduce<T, S>::ApplyMapReduce( std::function<S(S,S)> _reduce_func ) 
{
    merge_func = _reduce_func;

    for(size_t i = 0; i < threads.size(); i++)
    {
        threads[i] = std::thread(&OrderedMapAndReduce::MapReduce,this,i,threads.size());
    }

    for(size_t i = 0; i < threads.size();i++)
    {
        threads[i].join();
    }

    return proccessed_data[0];
}

template<typename T, typename S>
void OrderedMapAndReduce<T, S>::MapReduce(int index, const int thread_count)
{
    // apply map
    proccessed_data[index] = S(raw_data[index]);

    // reduce
    int merge_width = 1;
    bool finished = false;

    while (!finished && layer < std::log2(thread_count))
    {
        int map_index = thread_count - (thread_count >> layer) + (index / (merge_width << 1) );

        std::unique_lock<std::mutex> wrlock( mutexes[map_index] );
        
        if( (index/merge_width)%2 == 0 )
        {
            if( index + merge_width < proccessed_data.size() )
            {
                merge_check[map_index].wait( wrlock, [&] {return merge_active[map_index] == 1;});
                proccessed_data[index] = merge_func( proccessed_data[index], proccessed_data[index+merge_width]);
            }
        }
        else
        {
            merge_active[map_index] = 1;
            wrlock.unlock();
            merge_check[map_index].notify_one();
            finished = true;
        }
        
        merge_width <<= 1;
        layer++;
    }
}