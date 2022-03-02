#include "BoidManager.h"
#include <iostream>
#include <omp.h>
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

BoidManager::BoidManager(   int number_boids, glm::vec3 spawn_volume, float boid_initial_vel, float sim_radius, int num_threads, int _is_omp,
                            std::vector<glm::vec3>* _boid_colours , std::vector<glm::mat4>* _boid_matricies ):
gen(1),
boid_count(number_boids),
thread_count(num_threads),
simulation_radius(sim_radius),
thread_dimension(1,1,1),
boid_colours(_boid_colours),
boid_matricies(_boid_matricies),
last_clock_time(omp_get_wtime()),
thread_colours(num_threads),
is_omp(_is_omp)
{
    if( is_omp == 1 )
        omp_set_num_threads(num_threads);
    else
        thread_pool = std::make_unique<ThreadPoolAdv>(num_threads);

    // (x,y,z) where x,y,z are elements of {-1,0,1}
    for(int x = -1; x < 2; x++)
        for(int y = -1; y < 2; y++)
            for(int z = -1; z < 2; z++)
                neighbour_search.push_back( glm::ivec3(x,y,z) );

    // Boid Instantiation
    // ---------------------------------------------------------------------------------
    std::uniform_real_distribution<float> rand_x_pos(-spawn_volume[0],spawn_volume[0]);
    std::uniform_real_distribution<float> rand_y_pos(-spawn_volume[1],spawn_volume[1]);
    std::uniform_real_distribution<float> rand_z_pos(-spawn_volume[2],spawn_volume[2]);

    std::uniform_real_distribution<float> rand_vec(-1.0f,1.0f);

    for(int i = 0; i < boid_count; i++)  
    {
        glm::vec3 pos{ rand_x_pos(gen), rand_y_pos(gen), rand_z_pos(gen) };
        glm::vec3 vel{ rand_vec(gen), rand_vec(gen), rand_vec(gen) }; 
        vel = glm::normalize(vel) * boid_initial_vel;
        Boid* new_boid = new Boid(pos,vel,i);
        boids.emplace_back( new_boid );
    }

    AssignThreadsToBins();
}

void BoidManager::AssignThreadsToBins( )
{
    /*  Maps which proccessor will handle which grid spaces with the idea that grid spaces close to
        one another are proccessed by the same proccessor to improve data locality/ reduce false sharing */

    // n = log2(num_threads)
    int n = 0;
    int num_threads = thread_count;
    while ((num_threads >>= 1) > 1)
        n++;
    
    // generates a new coordinate system - thread_dimension
    int div_ctr = 0;
    for(;n >= 0; n--)
    {
        thread_dimension[div_ctr] *= 2;
        div_ctr = (div_ctr+1) % 3;
    }

    // assigns a processor to each unique coordinate in the system
    int thread_counter = 0;
    for(int x = 0; x < thread_dimension.x; x++)
        for(int y = 0; y < thread_dimension.y; y++)
            for(int z = 0; z < thread_dimension.z; z++)
                thread_lattice_map.emplace( glm::ivec3(x,y,z), thread_counter++ );
}

void BoidManager::ComputeBoidsSteering(glm::ivec3 dim) 
{
    /*  Calculates the steering for every boid in the grid block (dim).
        thread-safe if the same dim is not called by different processors. */

    std::vector<Boid*>& active_boid_segment = boids_partition[dim.x][dim.y][dim.z];
    
    // iterate through all boids in the grid block
    for(size_t i = 0; i < active_boid_segment.size(); i++)
    {
        Boid* active_boid = active_boid_segment[i];
        FindNeighbours( active_boid , dim );
        active_boid->ComputeBoundarySteer(simulation_radius);
        active_boid->ComputeSAC(); 

        // assign colours to boids based on which thread is assigned to it
        // if this is not a benchmark run
        if( boid_colours != nullptr )
        {
            if( is_omp == 1 )
                (*boid_colours)[active_boid->id] = thread_colours.colours[omp_get_thread_num()];
            else // thread colouring based on the initially assigned proccessor; in many cases may be executed by another thread - through work stealing
                (*boid_colours)[active_boid->id] = thread_colours.colours[thread_lattice_map[FindThreadLatticePosition(dim)]];
        }            
    }
}

void BoidManager::ApplyBoidsSteering(glm::ivec3 dim) 
{
    /*  Applies steering for every boid in the grid block (dim) and prepares boids for next iteration 
        thread-safe if the same dim is not called by different processors */
    
    std::vector<Boid*>& active_boid_segment = boids_partition[dim.x][dim.y][dim.z];
    
    // iterate through all boids in grid block
    for( size_t i = 0; i < active_boid_segment.size(); i++ )
    {
        Boid* active_boid = active_boid_segment[i];
        active_boid->ApplySteering(delta_time*100);
        active_boid->neighbours.clear();
        // move boid back to original vector - copying is expensive
        boids[active_boid->id] = active_boid;
        if( boid_matricies != nullptr )
            (*boid_matricies)[active_boid->id] = active_boid->GetMatrix();
    }
}

glm::ivec3 BoidManager::FindThreadLatticePosition(glm::ivec3 position)
{
    /* Maps the grid space coordinate to thread coordinate */
    
    // may be a nicer linear algbra way of doing this
    glm::ivec3 n_pos;
    for(int i = 0; i < 3; i++)
    {
        n_pos[i] = glm::min( (position[i]*thread_dimension[i]) / BOID_SEGMENTS_PER_DIM , thread_dimension[i]-1);
    }
    return n_pos;
}

glm::ivec3 BoidManager::FindLatticePosition(glm::vec3 position)
{
    /* maps world coordinate to grid space coordinate */

    int scaled_x = std::max(((position.x+simulation_radius)*BOID_SEGMENTS_PER_DIM)/(2*simulation_radius),0.0f );
    scaled_x = std::min( BOID_SEGMENTS_PER_DIM-1 , scaled_x );
    int scaled_y = std::max(((position.y+simulation_radius)*BOID_SEGMENTS_PER_DIM)/(2*simulation_radius),0.0f );
    scaled_y = std::min( BOID_SEGMENTS_PER_DIM-1 , scaled_y );
    int scaled_z = std::max(((position.z+simulation_radius)*BOID_SEGMENTS_PER_DIM)/(2*simulation_radius),0.0f );
    scaled_z = std::min( BOID_SEGMENTS_PER_DIM-1 , scaled_z );
    
    return glm::ivec3(scaled_x,scaled_y,scaled_z);
}

void BoidManager::PopulatePartition( )
{
    /* Place all boids in correct localised vector depending on their world space position */

    for(int i = 0; i < boid_count; i++ )
    {
        glm::ivec3 bin_pos = FindLatticePosition( boids[i]->position );
        boids_partition[bin_pos.x][bin_pos.y][bin_pos.z].push_back( boids[i]);//std::move(boids[i]) );
    }
}

void BoidManager::FindNeighbours( Boid* active_boid , glm::ivec3 active_bin ) 
{
    /*  Given a boid and grid space, assign to that boid all other boids that are within a given distance.
        thread safe as long as the same active boid is not called by different threads. */

    // iterate all neighbouring grid spaces
    for(size_t i = 0; i < neighbour_search.size(); i++)
    {
        glm::ivec3 n_index = neighbour_search[i] + active_bin;
        // check if in bounds
        if( n_index.x >= 0 && n_index.x < BOID_SEGMENTS_PER_DIM &&
            n_index.y >= 0 && n_index.y < BOID_SEGMENTS_PER_DIM &&
            n_index.z >= 0 && n_index.z < BOID_SEGMENTS_PER_DIM )
        {
            std::vector<Boid*>& search_bin = boids_partition[n_index.x][n_index.y][n_index.z];
            for(size_t j = 0; j < search_bin.size(); j++)
            {   
                // assign neighbour pointer if within a certain distance
                float distance = glm::distance( search_bin[j]->position, active_boid->position );
                if( glm::abs(distance) > BOID_EPSILON && distance < BoidAttributesShared::radius )
                    active_boid->neighbours.push_back( search_bin[j] );
            }
        }
    }
}

void BoidManager::UpdateLogicOMP( )
{
    /* Executes one iteration of the simulation using OpenMP*/

    PopulatePartition();
    delta_time = last_clock_time;
    last_clock_time = omp_get_wtime();
    delta_time = last_clock_time - delta_time;

    #pragma omp parallel for collapse(3) schedule(nonmonotonic:dynamic)
    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
    {
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
        {
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
            {
                ComputeBoidsSteering(glm::ivec3(x,y,z));
            }
        }    
    }

    #pragma omp parallel for collapse(3) schedule(nonmonotonic:dynamic)
    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
    {
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
        {
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
            {
                ApplyBoidsSteering(glm::ivec3(x,y,z));
            }
        }
    }

    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
                boids_partition[x][y][z].clear();
}

void BoidManager::UpdateLogicSTDThread() 
{
    /* Executes one iteration of the simulation using <thread>*/

    PopulatePartition();
    delta_time = last_clock_time;
    last_clock_time = omp_get_wtime();
    delta_time = last_clock_time - delta_time;

    int ctr = 0;
    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
    {
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
        {
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
            {
                glm::ivec3 dim(x,y,z);
                int p_id = thread_lattice_map[FindThreadLatticePosition(dim)];
                    thread_pool->AssignWorkToThread(
                        std::bind(&BoidManager::ComputeBoidsSteering,this,dim),
                        p_id
                    );
            } 
        }    
    }
    thread_pool->ExecuteWorkQueue();
    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
    {
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
        {
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
            {
                glm::ivec3 dim(x,y,z);
                int p_id = thread_lattice_map[FindThreadLatticePosition(dim)];
                    thread_pool->AssignWorkToThread(
                        std::bind(&BoidManager::ApplyBoidsSteering,this,dim),
                        p_id
                    );

            }
        }    
    }

    thread_pool->ExecuteWorkQueue();

    for(int x = 0; x < BOID_SEGMENTS_PER_DIM; x++)
        for(int y = 0; y < BOID_SEGMENTS_PER_DIM; y++)
            for(int z = 0; z < BOID_SEGMENTS_PER_DIM; z++)
                boids_partition[x][y][z].clear();
}

BoidManager::~BoidManager() 
{
    for(int i = 0; i < boid_count;i++)
        delete boids[i];
}
