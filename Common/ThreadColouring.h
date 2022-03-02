#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct ThreadColouring
{
    /* Randomly assign colours for the number of threads */
    std::vector<glm::vec3> colours;

    ThreadColouring( int num_threads )
    {
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_real_distribution<float> rand_col(0.0f,1.0f);

        for(int i = 0; i < num_threads; i++)
        {
            colours.push_back( glm::vec3( rand_col(gen), rand_col(gen), rand_col(gen) ) );
        }
    }
};