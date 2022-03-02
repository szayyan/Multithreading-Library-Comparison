#include "FlockSimulation.h"
#include <glm/glm.hpp>
#include <iostream>

FlockSimulation::FlockSimulation(ParallelData _data ):
data(_data),
boid_manager( std::make_unique<BoidManager>(_data.n,glm::vec3(50.0,50.0,50.0),0.1f,100.0f,_data.num_threads,_data.thread_or_omp) )
{

    std::cout << "Initialising Flocking Simulation" << std::endl <<
                "Boid Count: " << data.n << std::endl <<
                "Thread Count: " << data.num_threads << std::endl;

}

void FlockSimulation::Run() 
{
    /* execute 1000 iterations of flocking simulation */

    for(int i = 0; i < 1000; i++)
    {
        // branch prediction means overhead of conditional is neglibile - wont affect results
        // can use function pointers to elimate this but not neccassary
        if( data.thread_or_omp == 1 )
            boid_manager->UpdateLogicOMP();
        else
            boid_manager->UpdateLogicSTDThread();
    }
} 

void FlockSimulation::OutputStats() 
{
    std::cout << std::endl;
}

FlockSimulation::~FlockSimulation() 
{}

