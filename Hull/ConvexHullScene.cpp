#define GLM_ENABLE_EXPERIMENTAL
#include "ConvexHullScene.h"
#include "Primitives.h"
#include "Shader.h"
#include <random>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/constants.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <omp.h>


ConvexHullScene::ConvexHullScene(ParallelData _data):
data(_data),
point_colours(1.0f,1.0f,1.0f),
iter(0),
layer(0),
time_between_layer(1),
volume{ 750.0f, 400.0f },
last_time_drawn(0)
{
    GeneratePointsAndHull();
}

void ConvexHullScene::InitOpenGL(float ratio)
{
    // Glew initilisation
    // ---------------------------------------------------------------------------------
    glewExperimental = true;
    if( glewInit() != GLEW_OK )
    {
        std::cout << "GLEW Initialisation failed - exiting" << std::endl;
        return;
    }

    // OpenGL variables
    // ---------------------------------------------------------------------------------
    glClearColor(0.1,0.1,0.1,1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Shader Handling 
    // ---------------------------------------------------------------------------------
    std::vector<std::string> shader_source { "Shaders/simple.frag", "Shaders/simple.vert" };
    shader_program = Shader::LoadShaders( shader_source );
    glUseProgram( shader_program );

    glGenVertexArrays(1, &point_vao);
    glBindVertexArray( point_vao );
    
    Primitives::ModelData point_data = Primitives::Sphere(10,10,1.0f);
    index_count = point_data.indices.size();
    vert_count = point_data.vertex_positions.size();

    glGenBuffers(1, &point_vertex_attrib );
    glBindBuffer( GL_ARRAY_BUFFER, point_vertex_attrib );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vert_count, &point_data.vertex_positions[0], GL_STATIC_DRAW );

    glGenBuffers(1, &point_colour_attrib);
    glBindBuffer( GL_ARRAY_BUFFER, point_colour_attrib );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3), &point_colours[0], GL_STATIC_DRAW );

    glGenBuffers(1, &point_index_attrib );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, point_index_attrib );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * index_count , &point_data.indices[0], GL_STATIC_DRAW );
    
    glGenBuffers(1, &point_position_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, point_position_attrib);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * point_cloud.size() , &point_cloud[0], GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer( GL_ARRAY_BUFFER, point_vertex_attrib );
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*) 0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, point_position_attrib );
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*) 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, point_colour_attrib );
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    glVertexAttribDivisor(1, 1);
    //glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(2, point_cloud.size());

    glGenVertexArrays(1,&line_vao);
    glBindVertexArray(line_vao);

    glGenBuffers(1, &line_position_buffer);
    glGenBuffers(1, &line_colour_attrib);

    glEnableVertexAttribArray(0);
    glBindBuffer( GL_ARRAY_BUFFER, line_position_buffer );
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*) 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, line_colour_attrib );
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    //glVertexAttribDivisor(2,1);

    // Projection Maths
    // ---------------------------------------------------------------------------------
    glm::mat4 projection = glm::ortho(-500.0f*ratio,500.0f*ratio, -500.0f, 500.0f, 0.0f, 150.0f);////
    glm::mat4 view = glm::lookAt(   glm::vec3(0.0f,0.0f,50.0f),
                                    glm::vec3(0.0,0.0,0.0),
                                    glm::vec3(0.0,1.0,0.0) );

    // Pass Model View Projection matrix to our shader
    // ---------------------------------------------------------------------------------
    glm::mat4 tri_mvp = projection * view;
    GLint tri_mvp_id = glGetUniformLocation( shader_program, "mvp" );
    glUniformMatrix4fv(tri_mvp_id, 1, GL_FALSE, &tri_mvp[0][0]);
}


void ConvexHullScene::DrawHull(const QuickHull& hull, glm::vec3 colour)
{
    // the hull will be added to line_positions which is then rendered
    for(int i = 0; i < hull.edges.size(); i++)
    {
        line_positions.push_back( hull.edges[i].start );
        line_positions.push_back( hull.edges[i].start + hull.edges[i].direction );
        line_colours.push_back( colour );
        line_colours.push_back( colour );
    }
}

void ConvexHullScene::UpdateFrame()
{
    /* One frame of the chull scene */
    
    double elapsed_time = omp_get_wtime();
    // if enough time has passed since last draw next layer again
    if( elapsed_time - last_time_drawn >= time_between_layer )
    {   
        line_positions.clear();
        line_colours.clear();

        // regenerate point set and start again
        if( layer == hulls_to_draw.size() )
        {
            GeneratePointsAndHull();
            glBindBuffer(GL_ARRAY_BUFFER, point_position_attrib);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * point_cloud.size() , &point_cloud[0], GL_STREAM_DRAW);
            layer = 0;
        }

        last_time_drawn = elapsed_time;
        
        for(size_t i = 0; i < hulls_to_draw[layer].size(); i++)
        {
            DrawHull( hulls_to_draw[layer][i] , hull_colours[layer][i] );
        }

        // rebind data to GPU
        glBindVertexArray(line_vao);
        glBindBuffer( GL_ARRAY_BUFFER, line_position_buffer );
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * line_positions.size(), &line_positions[0], GL_STATIC_DRAW );

        glBindBuffer( GL_ARRAY_BUFFER, line_colour_attrib );
        glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * line_colours.size(), &line_colours[0], GL_STATIC_DRAW );

        layer++;

    }
    // draw lines and points
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(line_vao);
    glDrawArrays(GL_LINES,0,line_positions.size());
    glBindVertexArray(point_vao);
    glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, point_cloud.size() );
    
    
}
 
ConvexHullScene::~ConvexHullScene()
{
}

void ConvexHullScene::GeneratePointsAndHull() 
{
    point_cloud.clear();

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

    // execute convec hull algorithm
    ConvexHullParallel qhp;
    qhp.ConvexHullVisual(point_cloud, volume.x, data.num_threads);
    hulls_to_draw = qhp.intermediate_hulls;
    hull_colours = qhp.intermediate_colours;
}