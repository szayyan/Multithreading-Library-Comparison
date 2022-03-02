#include <SFML/Graphics.hpp>
#include "FlockSimulation.h"
#include "SceneManager.h"
#include <iostream>
#include <memory>
#include <limits>
#include <map>
#include <chrono>
#include <omp.h>
#include <string>
#include "Runnable.h"
#include "Scene.h"
#include "FlockSimulationScene.h"
#include "ParallelData.h"
#include "ConvexHullScene.h"
#include "BFSScene.h"
#include "ConvexHull.h"
#include "BFSRun.h"
#include "ArgParser.h"

int main(int argc, char* argv[])
{
    // fetch hardware concurrency
    int avaliable_threads = omp_get_max_threads();

    // default values for parameters that arent supplied
    std::map<std::string,Param> param_defaults
    {
        {"-threads", { avaliable_threads, 1, std::numeric_limits<int>::max() }},
        {"-lib", { 1, 1, 2 }},
        {"-benchmark", { 0, 0, 1 }},
        {"-algo", { 1, 1, 3 }},
        {"-n", { 0, 1, std::numeric_limits<int>::max()}},
        {"-out", { 1, 0, 1} }
    };

    ParseArgs(argc, argv, param_defaults, 50*avaliable_threads );

    ParallelData data { param_defaults["-n"].value,
                        param_defaults["-threads"].value,
                        param_defaults["-lib"].value };

    if( param_defaults["-benchmark"].value == 0 )
    {
        // visual stuff - begin scene
        std::unique_ptr<IScene> scene;
        switch ( param_defaults["-algo"].value )
        {
            case 1:
                scene = std::make_unique<FlockSimulationScene>(data);
                break;
            case 2:
                scene = std::make_unique<ConvexHullScene>(data);
                break;
            case 3:
                scene  = std::make_unique<BFSScene>(data);
                break;
        }
        auto manager = std::make_unique<SceneManager>(scene);
        manager->Run(); 
    }
    else if(param_defaults["-benchmark"].value == 1 )
    {
        // begin benchmark 
        std::unique_ptr<Runnable> program;
        switch ( param_defaults["-algo"].value )
        {
            case 1:
                program = std::make_unique<FlockSimulation>( data );
                break;
            case 2:
                program = std::make_unique<ConvexHull>( data );
                break;
            case 3:
                program = std::make_unique<BFSRun>( data );
                break;
        }

        // start benchmark run - will use GBenchmark later but for now this is just fine
        auto start_time = std::chrono::high_resolution_clock::now();
        program->Run();
        auto end_time = std::chrono::high_resolution_clock::now();

        auto run_time = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time );
        program->OutputStats();
        std::cout << std::endl << "Time taken to run: " << run_time.count() << "ms" << std::endl;
    }

    return 0;

}