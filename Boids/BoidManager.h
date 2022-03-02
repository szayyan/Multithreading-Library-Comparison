#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <thread>
#include <unordered_map>
#include "Boid.h"
#include "ThreadColouring.h"
#include "VecHash.h"
#include "ThreadPoolAdv.h"

#define BOID_SEGMENTS_PER_DIM 6
#define BOID_EPSILON 1e-4

class BoidManager
{
public:
    BoidManager(int number_boids, glm::vec3 spawn_volume, float boid_initial_vel, float sim_radius, int num_threads, int is_omp,
                std::vector<glm::vec3>* _boid_colours = nullptr, std::vector<glm::mat4>* _boid_matricies = nullptr );
    void UpdateLogicOMP();
    void UpdateLogicSTDThread();
    ~BoidManager();

private:
    // stores the time in ms between each frame/iteration
    float delta_time;

    // exact time of last frame / iteration
    float last_clock_time;

    // max distance in which boids are allowed to roam
    float simulation_radius;
    
    int thread_count;
    int boid_count;
    int is_omp;

    // used for random number gen
    std::random_device rd;
    std::mt19937 gen;

    // container for all boids in scene
    std::vector<Boid*> boids;

    // stores copy of boid within localised array. Boids in similar world space are placed in the same array 
    std::vector<Boid*> boids_partition[BOID_SEGMENTS_PER_DIM][BOID_SEGMENTS_PER_DIM][BOID_SEGMENTS_PER_DIM];

    // mapping from position in partition to worker. Adjacent grid blocks in thread space are mapped to the 
    // same processor to improve data locality and reduce cache misses
    std::unordered_map<glm::ivec3,int> thread_lattice_map;

    // dimensions of thread space - explained in detail in report
    glm::ivec3 thread_dimension;

    // array containing all directions from which a grid block should be searched.
    std::vector<glm::ivec3> neighbour_search;

    ThreadColouring thread_colours; 
    std::vector<glm::vec3>* boid_colours;
    std::vector<glm::mat4>* boid_matricies;
    std::unique_ptr<ThreadPoolAdv> thread_pool;

    void FindNeighbours(Boid* active_boid , glm::ivec3 active_bin );
    void PopulatePartition( );
    void AssignThreadsToBins( );
    void ComputeBoidsSteering(glm::ivec3 dim );
    void ApplyBoidsSteering( glm::ivec3 dim );
    glm::ivec3 FindLatticePosition(glm::vec3 position);
    glm::ivec3 FindThreadLatticePosition(glm::ivec3 position);
};