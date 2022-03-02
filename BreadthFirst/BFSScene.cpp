#define GLM_ENABLE_EXPERIMENTAL
#include "BFSScene.h"
#include <glm/gtx/transform.hpp>
#include "Shader.h"
#include "Primitives.h"
#include <omp.h>
#include <algorithm>

BFSScene::BFSScene(ParallelData _data):
data(_data),
last_time_updated(omp_get_wtime()),
time_between_updates(0.5f),
spatial_graph( Graph3D::GenerateRandomGraph(data.n,50,glm::vec3(400.0,400.0,400.0),0.45) ),
thread_colours(data.num_threads),
cube_colours( data.n, glm::vec3(0.7,0.7,0.7) ),
line_colours(),
edges_drawn(),
bfs( std::make_unique<BFSVisualised>(   std::make_shared<Graph>(spatial_graph->graph),
                                        20,
                                        data,
                                        cube_colours,
                                        thread_colours.colours,
                                        line_colours,
                                        edges_drawn) )
{
    UpdateEdgeData();
}

void BFSScene::InitOpenGL(float ratio) 
{
    // glew initilisation
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

    // Passing data to the GPU
    // ---------------------------------------------------------------------------------

    Primitives::ModelData cube = Primitives::Sphere(5,5,1.0f);
    index_count = cube.indices.size();

    glGenVertexArrays( 1, &VAO);
    glBindVertexArray( VAO );

    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube.vertex_positions.size() , &cube.vertex_positions[0], GL_STATIC_DRAW);

    glGenBuffers(1,&node_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, node_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * spatial_graph->vertex_positions.size(), &spatial_graph->vertex_positions[0], GL_STATIC_DRAW);

    glGenBuffers(1,&cube_color_attribute);
    glBindBuffer(GL_ARRAY_BUFFER, cube_color_attribute);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cube_colours.size(), &cube_colours[0], GL_STREAM_DRAW);
    
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(int) * index_count, &cube.indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, node_position_buffer);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);      
    glVertexAttribDivisor(1,1);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,cube_color_attribute);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glVertexAttribDivisor(2,1);

    glGenVertexArrays( 1, &lineVAO);
    glBindVertexArray( lineVAO );

    glGenBuffers(1, &lineVBO);
    glBindBuffer( GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*line_verts.size(), &line_verts[0],GL_STATIC_DRAW);

    glGenBuffers(1,&line_color_attribute);
    glBindBuffer(GL_ARRAY_BUFFER,line_color_attribute);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*line_colours.size(), &line_colours[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, line_color_attribute);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    // Projection Maths
    // ---------------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 2000.0f);//glm::ortho(  -400.0, 400.0, -400.0, 400.0,-400.0, 1000.0);//
    glm::mat4 view = glm::lookAt(   glm::vec3(300.0f,300.0f,300.0f),
                                    glm::vec3(0.0,0.0,0.0),
                                    glm::vec3(0.0,1.0,0.0) );


    // Pass Model View Projection matrix to our shader
    // ---------------------------------------------------------------------------------
    glm::mat4 tri_mvp = projection * view;
    GLint tri_mvp_id = glGetUniformLocation( shader_program, "mvp" );
    glUniformMatrix4fv(tri_mvp_id, 1, GL_FALSE, &tri_mvp[0][0]);
}

void BFSScene::UpdateFrame() 
{
    double elapsed_time = omp_get_wtime();
    // every (time_between_updates) seconds this branch will become true
    if( elapsed_time - last_time_updated >= time_between_updates )
    {
        last_time_updated = elapsed_time;

        // apply one iteration of bfs
        // branch is true means search has finished
        if( bfs->Iterate() )
        {
            // generate new graph and search class
            spatial_graph = Graph3D::GenerateRandomGraph(data.n,50,glm::vec3(400.0,400.0,400.0),0.45);
            bfs = std::make_unique<BFSVisualised>(  std::make_shared<Graph>(spatial_graph->graph),
                                                    20,
                                                    data,
                                                    cube_colours,
                                                    thread_colours.colours,
                                                    line_colours,
                                                    edges_drawn);
            
            // reset drawing variables
            std::fill(cube_colours.begin(),cube_colours.end(),glm::vec3(0.7,0.7,0.7));
            line_colours.clear();
            line_verts.clear();
            edges_drawn.clear();

            UpdateEdgeData();

            // rebind data and pass to the GPU
            glBindBuffer(GL_ARRAY_BUFFER, node_position_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * spatial_graph->vertex_positions.size(), &spatial_graph->vertex_positions[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, cube_color_attribute);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cube_colours.size(), &cube_colours[0], GL_STREAM_DRAW);
            glBindBuffer( GL_ARRAY_BUFFER, lineVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*line_verts.size(), &line_verts[0],GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER,line_color_attribute);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*line_colours.size(), &line_colours[0], GL_STATIC_DRAW);
        }
        else
        {
            // update colours to correct values
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, cube_color_attribute);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cube_colours.size(), &cube_colours[0], GL_STREAM_DRAW);

            glBindVertexArray(lineVAO);
            glBindBuffer(GL_ARRAY_BUFFER,line_color_attribute);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*line_colours.size(), &line_colours[0], GL_STATIC_DRAW);
        }
    }

    // draw vertices and edges
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES,index_count,GL_UNSIGNED_INT,(void*)0,spatial_graph->vertex_positions.size());
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES,0,line_verts.size());      

}

void BFSScene::UpdateEdgeData() 
{
    // iterates through given graph and populatues line_verts with 
    // edges that are connected in the graph
    for(uint i = 0; i < spatial_graph->graph.adjacency_list.size(); ++i)
    {
        for(uint j = 0; j < spatial_graph->graph.adjacency_list[i].size(); ++j)
        {
            int node_connect_indices = spatial_graph->graph.adjacency_list[i][j];
            DirectedEdge next_edge{ i, node_connect_indices};
            bool skip_edge = false;

            // used to prevent drawing the same edge twice
            for(uint x = 0; x < edges_drawn.size(); x++)
            {
                if(edges_drawn[x] == next_edge)
                {
                    skip_edge = true;
                    break;
                }
            }
            if(!skip_edge)
            {
                // store edge data to draw later
                edges_drawn.push_back(next_edge);            
                line_verts.push_back( spatial_graph->vertex_positions[i] );
                line_verts.push_back( spatial_graph->vertex_positions[ node_connect_indices ]);
                line_colours.push_back( glm::vec3(1.0,1.0,1.0));
                line_colours.push_back( glm::vec3(1.0,1.0,1.0));
            }
        }
    }
}

BFSScene::~BFSScene() 
{
    
}
