#define GLM_ENABLE_EXPERIMENTAL
#include "FlockSimulationScene.h"
#include "Shader.h"
#include "Primitives.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>


FlockSimulationScene::FlockSimulationScene( ParallelData _data ):
data( _data ),
boid_count( _data.n ),
boid_matricies( _data.n ),
boid_colours( _data.n ),
boid_manager( std::make_unique<BoidManager>(_data.n,
                                            glm::vec3(50.0,50.0,50.0), // initial spawn vol
                                            0.1f,
                                            100.0f,
                                            _data.num_threads,
                                            _data.thread_or_omp,
                                            &boid_colours,
                                            &boid_matricies ) )
{ 
    // set update fuction depending on user input
    update_func = std::bind(data.thread_or_omp == 1 ? &BoidManager::UpdateLogicOMP : &BoidManager::UpdateLogicSTDThread,
                                                boid_manager.get() );
}

void FlockSimulationScene::InitOpenGL( float ratio )
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
    std::vector<std::string> shader_source { "Shaders/boid.frag", "Shaders/boid.vert" };
    shader_program = Shader::LoadShaders( shader_source );
    glUseProgram( shader_program );

    // Pass vertex/ object data to the GPU
    // ---------------------------------------------------------------------------------
    glGenVertexArrays(1, &boid_vao);
    glBindVertexArray( boid_vao );

    // load model data
    Primitives::ModelData ship_data = Primitives::Ship( glm::vec3(1.0f,1.0f,3.0f), 1.0f );
    vert_count = ship_data.vertex_positions.size();

    glGenBuffers(1, &boid_vertex_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, boid_vertex_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vert_count, &ship_data.vertex_positions[0], GL_STATIC_DRAW );

    glGenBuffers(1, &boid_colour_attribute);
    glBindBuffer( GL_ARRAY_BUFFER, boid_colour_attribute );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * boid_colours.size(), &boid_colours[0], GL_STATIC_DRAW );

    glGenBuffers(1, &normal_buffer);
    glBindBuffer( GL_ARRAY_BUFFER, normal_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * ship_data.normals.size(), &ship_data.normals[0], GL_STATIC_DRAW );
 
    glGenBuffers(1,&boid_transformation_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, boid_transformation_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * boid_count , &boid_matricies[0], GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer( GL_ARRAY_BUFFER, boid_vertex_buffer );
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*) 0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, boid_colour_attribute );
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer );
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    size_t vec_size = sizeof(glm::vec4);
    
    glBindBuffer( GL_ARRAY_BUFFER, boid_transformation_buffer );
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec_size, (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec_size, (void*)(vec_size));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec_size, (void*)(2 * vec_size));
    glEnableVertexAttribArray(6); 
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec_size, (void*)(3 * vec_size));

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    // Projection Maths
    // ---------------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 20000.0f);
    glm::mat4 view = glm::lookAt(   glm::vec3(0.0f,200.0f,200.0f),
                                    glm::vec3(0.0,0.0,0.0),
                                    glm::vec3(0.0,1.0,0.0) );

    // Pass data to our shader for point lighting
    // ---------------------------------------------------------------------------------
    glm::mat4 tri_mvp = projection * view;
    GLint tri_mvp_id = glGetUniformLocation( shader_program, "mvp" );
    glUniformMatrix4fv(tri_mvp_id, 1, GL_FALSE, &tri_mvp[0][0]);

    GLint view_shader_id = glGetUniformLocation( shader_program, "view" );
    glUniformMatrix4fv(view_shader_id, 1, GL_FALSE, &view[0][0]);

    glm::vec3 light_position( 0.0, 0.0, 0.0);
    GLint light_pos_id = glGetUniformLocation( shader_program, "light_pos");
    glUniform3f( light_pos_id, light_position[0], light_position[1], light_position[2]);
    
}



void FlockSimulationScene::UpdateFrame()
{
    update_func();

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rebind boid position and colour data every frame
    glBindBuffer(GL_ARRAY_BUFFER, boid_transformation_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * boid_count , &boid_matricies[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, boid_colour_attribute);
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * boid_colours.size(), &boid_colours[0], GL_STREAM_DRAW );
    
    // draw boids
    glBindVertexArray(boid_vao);
    glDrawArraysInstanced( GL_TRIANGLES, 0 ,vert_count ,boid_count);
} 

FlockSimulationScene::~FlockSimulationScene()
{
    // glDeleteProgram( shader_program );
}