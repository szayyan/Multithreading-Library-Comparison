#include "ConvexHull.h"
#include "ConvexHullParallel.h"
#include <random>
#include <iostream>

ConvexHull::ConvexHull(ParallelData _data ):
data(_data),
volume{ 200.0f, 100.0f }
{
    // generate random point cloud within rectangular volume
    std::random_device rand;
    std::mt19937 gen(rand());
    std::uniform_real_distribution<float> x_dis(-volume.x,volume.x);
    std::uniform_real_distribution<float> y_dis(-volume.y,volume.y);

    for(int i = 0; i < data.n; i++)
    {
        glm::vec2 pos(x_dis(gen), y_dis(gen));
        point_cloud.push_back(pos);
    }

    std::cout << "Initialising Convex Hull Algorithm" << std::endl <<
                 "Point Cloud Size: " << data.n << std::endl <<
                 "Thread Count: " << data.num_threads << std::endl;   
}

void ConvexHull::Run()
{
    /* Run benchmark */
    ConvexHullParallel qhp;
    QuickHull result;
    
    if( data.thread_or_omp == 1 )
        result = qhp.ConvexHullOMP( point_cloud, volume.x, data.num_threads );
    else
        result = qhp.ConvexHullSTDThread( point_cloud, volume.x, data.num_threads );  
}

void ConvexHull::OutputStats() 
{}

ConvexHull::~ConvexHull() 
{}