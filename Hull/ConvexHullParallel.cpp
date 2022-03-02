#include "ConvexHullParallel.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <cmath>
#include <omp.h>
#include <thread>
#include "OrderedMapAndReduce.h"
#include "OrderedMapAndReduceLayer.h"
#include <algorithm>

ConvexHullParallel::ConvexHullParallel():
pos_colours(0),
intermediate_hulls(),
intermediate_colours()
{} 

QuickHull ConvexHullParallel::MapAndReduce( const std::vector<std::vector<glm::vec2>>& points )
{
    /* Idealistic map and reduce implementation using OpenMP */
    QuickHull final_hull;

    #pragma omp declare reduction ( hull_merge: \
                                    QuickHull: \
                                    omp_out = QuickHull::MergeHulls(omp_in,omp_out) \
                                    ) initializer(omp_priv=omp_orig)

    #pragma omp parallel for reduction(hull_merge:final_hull)
    for(size_t i = 0; i < points.size(); i++)
    {
        final_hull = QuickHull::MergeHulls( final_hull, QuickHull(points[i]) );
    }
    
    return final_hull;
}


std::vector<std::vector<glm::vec2>> ConvexHullParallel::PartionPoints(const std::vector<glm::vec2>& point_cloud,float x_max, int number_partitions) 
{
    /* Partitions the point cloud into equally distributed chunks */
    float split_size = (x_max*2)/number_partitions;
    std::vector<std::vector<glm::vec2>> partitioned_points(number_partitions);

    for(size_t i = 0; i < point_cloud.size(); i++)
    {
        float x_adjusted = point_cloud[i].x + x_max;
        int bin = (x_adjusted/(2*x_max)) * number_partitions;
        bin = bin == number_partitions ? number_partitions - 1 : bin;
        partitioned_points[bin].push_back(point_cloud[i]);
    }

    return partitioned_points;
}


QuickHull ConvexHullParallel::ConvexHullVisual( std::vector<glm::vec2>& point_cloud, float x_max, int num_threads ) 
{
    /* Applies the convexhull algorithm with all intermediate hulls returned so that the program can be visualised */
    omp_set_num_threads(num_threads);
    if( pos_colours.colours.size() != num_threads )
        pos_colours = ThreadColouring(num_threads);

    std::vector<std::vector<glm::vec2>> partitioned_points = PartionPoints(point_cloud,x_max,num_threads);
    std::vector<QuickHull> hulls = GenerateHullsParallelVisual(partitioned_points);
    intermediate_hulls.push_back(hulls);

    return MergeHullsParallelVisual(hulls,num_threads );
}


QuickHull ConvexHullParallel::ConvexHullOMP( std::vector<glm::vec2>& point_cloud, float x_max, int num_threads )
{
    /* Applies convex hull algorithm in parallel using OpenMP */
    omp_set_num_threads(num_threads);
    if( pos_colours.colours.size() != num_threads )
        pos_colours = ThreadColouring(num_threads);

    std::vector<std::vector<glm::vec2>> partitioned_points = PartionPoints(point_cloud,x_max,num_threads);

    // map in parallel
    std::vector<QuickHull> hulls = GenerateHullsParallel(partitioned_points);
    // reduce
    return MergeHullsParallel(hulls,num_threads );
}

QuickHull ConvexHullParallel::ConvexHullSTDThread( std::vector<glm::vec2>& point_cloud, float x_max, int num_threads ) 
{   
    /* Applies convex hull algorithm in parallel using C++ threading */
    if( pos_colours.colours.size() != num_threads )
        pos_colours = ThreadColouring(num_threads);

    std::vector<std::vector<glm::vec2>> partitioned_points = PartionPoints(point_cloud,x_max,num_threads);

    //OrderedMapAndReduce<glm::vec2,QuickHull> mpr(num_threads,partitioned_points);
    //return mpr.ApplyMapReduce( &QuickHull::MergeHulls );
   
    OrderedMapAndReduceLayer<glm::vec2,QuickHull> mpr(num_threads,partitioned_points);
    
    mpr.ApplyMapReduce( &QuickHull::MergeHulls ); 
    mpr.MapReduce(num_threads-1,num_threads);
    return mpr.FinaliseMerge();
}

 
std::vector<QuickHull> ConvexHullParallel::GenerateHullsParallel( std::vector<std::vector<glm::vec2>>& points )
{
    // simple openmp map operation applied to points vector
    size_t point_size = points.size();
    std::vector<QuickHull> hulls(point_size);

    #pragma omp parallel for
    for(size_t i = 0; i < points.size(); i++)
    {
        hulls[i] = QuickHull(points[i]);    
    }
    return hulls;
}

std::vector<QuickHull> ConvexHullParallel::GenerateHullsParallelVisual(std::vector<std::vector<glm::vec2>>& points) 
{
    /* same as above GenerateHullsParallel with colours specified */
    size_t point_size = points.size();
    std::vector<QuickHull> hulls(point_size);
    std::vector<glm::vec3> hull_colours(point_size);

    #pragma omp parallel for
    for(size_t i = 0; i < points.size(); i++)
    {
        hulls[i] = QuickHull(points[i]);    
        hull_colours[i] = pos_colours.colours[omp_get_thread_num()];
    }
    intermediate_colours.push_back(hull_colours);
    return hulls;
}

QuickHull ConvexHullParallel::MergeHullsParallel(std::vector<QuickHull> hulls, int thread_count ) 
{
    /* OpenMP reduction */
    
     // max depth is the largest int above thread count that is a power of two
    int max_depth = 1;
    while (max_depth < thread_count)
        max_depth <<= 1;

    // binary tree reduction
    #pragma omp parallel default(none) shared(hulls) firstprivate(thread_count,max_depth)
    {
        // begins at width 1 - neighbouring cells are merged
        int merge_width = 1;
        bool finished = false;
        int thread_num = omp_get_thread_num();

        // keeps iterating until reduction is complete
        while( merge_width < max_depth )
        {
            // odd/even merge
            if( (thread_num/merge_width)%2 == 0 && !finished)
            {
                // write back to original cell with computed value
                if( thread_num+merge_width < thread_count )
                    hulls[thread_num] = QuickHull::MergeHulls( hulls[thread_num], hulls[thread_num+merge_width]);
            }
            else
            {
                finished = true;
            }

            merge_width <<= 1;

            // synchronise at every layer of the tree
            #pragma omp barrier
        }        
    }

    return hulls[0];
}


QuickHull ConvexHullParallel::MergeHullsParallelVisual(std::vector<QuickHull> hulls, int thread_count ) 
{
    int max_depth = 1;
    while (max_depth < thread_count)
        max_depth <<= 1;

    intermediate_hulls.resize( std::log2(max_depth) + 1);
    intermediate_colours.resize( std::log2(max_depth) + 1);
    
    #pragma omp parallel
    {
        int merge_width = 1;
        bool finished = false;
        int thread_num = omp_get_thread_num();

        while( merge_width < max_depth )
        {
            if( (thread_num/merge_width)%2 == 0 && !finished )
            {
                if( thread_num+merge_width < thread_count )
                {
                    hulls[thread_num] = QuickHull::MergeHulls( hulls[thread_num], hulls[thread_num+merge_width]);
                    
                    // needed to prevent data race
                    #pragma omp critical
                    {
                        int layer = std::log2(merge_width) + 1;
                        intermediate_hulls[layer].push_back( hulls[thread_num] );
                        intermediate_colours[layer].push_back( pos_colours.colours[omp_get_thread_num()]);
                    }
                }
                else
                {
                    #pragma omp critical
                    {
                        int layer = std::log2(merge_width) + 1;
                        intermediate_hulls[layer].push_back( hulls[thread_num] );
                        intermediate_colours[layer].push_back( pos_colours.colours[omp_get_thread_num()]);
                    }
                }
            }
            else
            {
                finished = true;
            }

            merge_width <<= 1;

            #pragma omp barrier
        }        
    }
    return hulls[0];
}

