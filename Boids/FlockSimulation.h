#include "Runnable.h"
#include "BoidManager.h"
#include "ParallelData.h"
#include <memory>


class FlockSimulation : public Runnable
{
    /* Runs flock simulation benchmark */ 
    
    public:
        FlockSimulation( ParallelData _data );
        void Run();
        void OutputStats();
        ~FlockSimulation( );
    private:
        int boid_count;
        int thread_count;
        int threading_type;

        std::unique_ptr<BoidManager> boid_manager;
        ParallelData data;
};