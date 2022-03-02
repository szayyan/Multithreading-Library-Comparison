#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <iostream>

namespace Primitives
{
    /* Contains simple mesh data for rendering */
    
    struct ModelData
    {
        std::vector<float> vertex_positions;
        std::vector<int> indices;
        std::vector<float> normals;
    };

    // generates sphere model data
    inline ModelData Sphere(int sector_count, int stack_count, float radius )
    {
        std::vector<float> vertices;
        std::vector<int> inds;
        
        float sector_step = 2 * glm::pi<float>() / sector_count;
        float stack_step = glm::pi<float>() / stack_count;
        float sector_angle;
        float stack_angle;

        for(int i = 0; i <= stack_count; ++i)
        {
            stack_angle = glm::pi<float>() / 2 - i * stack_step;
            float z = glm::sin(stack_angle) * radius;

            for(int j = 0; j <= sector_count; ++j)
            {
                sector_angle = j * sector_step;           

                float x = glm::cos(sector_angle) * glm::cos(stack_angle) * radius;             
                float y = glm::sin(sector_angle) * glm::cos(stack_angle) * radius; 
                vertices.insert(vertices.end(),{x, y, z});
            }
        }

        int stack_index_1, stack_index_2;
        for(int i = 0; i < stack_count; ++i)
        {
            stack_index_1 = i * (sector_count + 1);
            stack_index_2 = stack_index_1 + sector_count + 1;

            for(int j = 0; j < sector_count; ++j, ++stack_index_1, ++stack_index_2)
            {
                if(i != 0)
                    inds.insert( inds.end(), {stack_index_1,stack_index_2,stack_index_1+1} );
                if(i != (stack_count-1))
                    inds.insert( inds.end(), {stack_index_1+1,stack_index_2,stack_index_2+1});

            }
        }

        ModelData sphere;
        sphere.vertex_positions = vertices;
        sphere.indices = inds;
        return sphere;
    }

    inline ModelData Ship( glm::vec3 vol_top, float extension )
    {
        std::vector<float> ship_vert_data
        {
            -0.700000, 0.000000, 0.700000, // 0
            0.700000, 0.000000, 0.700000, // 1
            0.000000, 2.500000, 0.000000,
            -0.700000, 0.000000, -0.700000, // 2
            0.000000, 2.500000, 0.000000,
            0.700000, 0.000000, -0.700000,
            0.700000, 0.000000, -0.700000,  // 3
            0.000000, 2.500000, 0.000000,   // 4
            0.700000, 0.000000, 0.700000,
            0.000000, 2.500000, 0.000000,
            -0.700000, 0.000000, -0.700000,
            -0.700000, 0.000000, 0.700000,
            -0.700000, 0.000000, -0.700000,
            0.700000, 0.000000, -0.700000,
            0.000000, -0.600000, -0.000000, // 5
            0.700000, 0.000000, 0.700000,
            -0.700000, 0.000000, 0.700000,
            0.000000, -0.600000, -0.000000,
            0.000000, -0.600000, -0.000000,
            -0.700000, 0.000000, 0.700000,
            -0.700000, 0.000000, -0.700000,
            0.000000, -0.600000, -0.000000,
            0.700000, 0.000000, -0.700000,
            0.700000, 0.000000, 0.700000
        };

        std::vector<int> indices
        {
            0,1,4,
            2,4,3,
            3,4,1,
            4,2,0,
            2,3,5,
            1,0,5,
            5,0,2,
            5,3,1
        };

        std::vector<float> normals
        {
            0.0000, 0.2696, 0.9630,
            0.0000, 0.2696, 0.9630,
            0.0000, 0.2696, 0.9630,
            0.0000, 0.2696, -0.9630, // correct normals
            0.0000, 0.2696, -0.9630,
            0.0000, 0.2696, -0.9630,
            0.9630, 0.2696, 0.0000,
            0.9630, 0.2696, 0.0000,
            0.9630, 0.2696, 0.0000,
            -0.9630, 0.2696, 0.0000,
            -0.9630, 0.2696, 0.0000,
            -0.9630, 0.2696, 0.0000,
            0.0000, -0.7593, -0.6508,
            0.0000, -0.7593, -0.6508,
            0.0000, -0.7593, -0.6508,
            0.0000, -0.7593, 0.6508,
            0.0000, -0.7593, 0.6508,
            0.0000, -0.7593, 0.6508,
            -0.6508, -0.7593, 0.0000,
            -0.6508, -0.7593, 0.0000,
            -0.6508, -0.7593, 0.0000,
            0.6508, -0.7593, 0.0000,
            0.6508, -0.7593, 0.0000,
            0.6508, -0.7593, 0.0000, 
        };

        ModelData ship_model;
        ship_model.vertex_positions = ship_vert_data;
        ship_model.normals = normals;
        return ship_model;
    }

    inline ModelData Cube()
    {
        std::vector<float> cube_vert_data
        {
            -1.000000, 1.000000, -1.000000,
            1.000000, 1.000000, 1.000000,
            1.000000, 1.000000, -1.000000,
            1.000000, 1.000000, 1.000000,
            -1.000000, -1.000000, 1.000000,
            1.000000, -1.000000, 1.000000,
            -1.000000, 1.000000, 1.000000,
            -1.000000, -1.000000, -1.000000,
            -1.000000, -1.000000, 1.000000,
            1.000000, -1.000000, -1.000000,
            -1.000000, -1.000000, 1.000000,
            -1.000000, -1.000000, -1.000000,
            1.000000, 1.000000, -1.000000,
            1.000000, -1.000000, 1.000000,
            1.000000, -1.000000, -1.000000,
            -1.000000, 1.000000, -1.000000,
            1.000000, -1.000000, -1.000000,
            -1.000000, -1.000000, -1.000000,
            -1.000000, 1.000000, -1.000000,
            -1.000000, 1.000000, 1.000000,
            1.000000, 1.000000, 1.000000,
            1.000000, 1.000000, 1.000000,
            -1.000000, 1.000000, 1.000000,
            -1.000000, -1.000000, 1.000000,
            -1.000000, 1.000000, 1.000000,
            -1.000000, 1.000000, -1.000000,
            -1.000000, -1.000000, -1.000000,
            1.000000, -1.000000, -1.000000,
            1.000000, -1.000000, 1.000000,
            -1.000000, -1.000000, 1.000000,
            1.000000, 1.000000, -1.000000,
            1.000000, 1.000000, 1.000000,
            1.000000, -1.000000, 1.000000,
            -1.000000, 1.000000, -1.000000,
            1.000000, 1.000000, -1.000000,
            1.000000, -1.000000, -1.000000
        };

        std::vector<float> normals
        {
            0.0000, 1.0000, 0.0000,
            0.0000, 1.0000, 0.0000,
            0.0000, 1.0000, 0.0000,
            0.0000, 0.0000, 1.0000,
            0.0000, 0.0000, 1.0000,
            0.0000, 0.0000, 1.0000,
            -1.0000, 0.0000, 0.0000,
            -1.0000, 0.0000, 0.0000,
            -1.0000, 0.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            0.0000, 0.0000, -1.0000,
            0.0000, 0.0000, -1.0000,
            0.0000, 0.0000, -1.0000,
            0.0000, 1.0000, 0.0000,
            0.0000, 1.0000, 0.0000,
            0.0000, 1.0000, 0.0000,
            0.0000, 0.0000, 1.0000,
            0.0000, 0.0000, 1.0000,
            0.0000, 0.0000, 1.0000,
            -1.0000, 0.0000, 0.0000,
            -1.0000, 0.0000, 0.0000,
            -1.0000, 0.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            0.0000, -1.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            1.0000, 0.0000, 0.0000,
            0.0000, 0.0000, -1.0000,
            0.0000, 0.0000, -1.0000,
            0.0000, 0.0000, -1.0000
        };

        ModelData cube_data;
        cube_data.vertex_positions = cube_vert_data;
        cube_data.normals = normals;

        return cube_data;
    }
}