#pragma once
#include "Graph.h"
#include <memory>
#include <random>
#include <iostream>
#include <algorithm>
#include <set>

class Graph3D
{
    /*  Represents a 3 dimensional network - a graph where each vertex is also represented by 
        a physical point in 3D space */
public:

    Graph graph;
    std::vector<glm::vec3> vertex_positions;

    Graph3D( const Graph& graph_obj, const std::vector<glm::vec3>& v_positions ):
    graph(graph_obj),
    vertex_positions(v_positions)
    {}

    float FindDistance( int vertex1, int vertex2 )
    {
        return glm::distance( vertex_positions[vertex1], vertex_positions[vertex2] );
    }
    
    static std::shared_ptr<Graph3D> GenerateRandomGraph( int number_of_verticies, float max_edge_dist, glm::vec3 bounding_volume, float connect_probability )
    {
        /*  Method 1 for random graph gen - distance based
            for each vertex a random position is generated.
            If another vertex is within max_edge_dist an edge may be formed
            with this vertex depending on the probability connect_probability  */

        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_real_distribution<float> x_dis( -bounding_volume[0]/2, bounding_volume[0]/2 );
        std::uniform_real_distribution<float> y_dis( -bounding_volume[1]/2, bounding_volume[1]/2 );
        std::uniform_real_distribution<float> z_dis( -bounding_volume[2]/2, bounding_volume[2]/2 );
        std::uniform_real_distribution<float> con_prob( 0.0 , 1.0 );

        Graph graph( number_of_verticies );

        std::vector<glm::vec3> v_pos;

        for(int i = 0; i < number_of_verticies; i++)
        {
            glm::vec3 pos { x_dis(gen), y_dis(gen), z_dis(gen) };
            for( int j = 0; j < v_pos.size(); j++ )
            {
                if( glm::distance(v_pos[j],pos) < max_edge_dist &&
                    con_prob(gen) > connect_probability)
                {
                    graph.AddEdge( DirectedEdge{i,j} );
                    graph.AddEdge( DirectedEdge{j,i} );
                }
            }
            v_pos.push_back( pos);
        }
        return std::make_shared<Graph3D>(graph,v_pos);
    }

    static std::shared_ptr<Graph3D> GenerateRandomGraphFast( int number_vertices, glm::vec3 volume, int max_edges, float p )
    {
        /*  Method 2 for random graph gen 
            random graph is initially generated using the graph class
            vertex positions are then assigned at random 
            fast to produce graph but visually weak */

        std::random_device rd;
        std::default_random_engine gen(1);
        std::uniform_real_distribution<float> x_dis( -volume[0]/2, volume[0]/2 );
        std::uniform_real_distribution<float> y_dis( -volume[1]/2, volume[1]/2 );
        std::uniform_real_distribution<float> z_dis( -volume[2]/2, volume[2]/2 );
        
        std::binomial_distribution<int> con_prob( max_edges , p );

        Graph gr = Graph::GenerateRandomGraph( number_vertices, max_edges, p );
        std::vector<glm::vec3> v_pos;

        for(int i = 1; i < number_vertices; i++)
            v_pos.push_back( glm::vec3( x_dis(gen), y_dis(gen), z_dis(gen) ));

        return std::make_shared<Graph3D>(gr, v_pos );
    }

    /*static std::shared_ptr<Graph3D> GenerateRandomGraphEdgeBased( int number_vertices, glm::vec3 volume, float no_edges_avg )
    {

        Graph gr(number_vertices);
        std::random_device rd;
        std::default_random_engine gen(1);
        std::uniform_real_distribution<float> x_dis( -volume[0]/2, volume[0]/2 );
        std::uniform_real_distribution<float> y_dis( -volume[1]/2, volume[1]/2 );
        std::uniform_real_distribution<float> z_dis( -volume[2]/2, volume[2]/2 );
        
        std::uniform_int_distribution<int> rand_vertex( 0 , number_vertices-1 );
        std::vector<DirectedEdge> all_edges;
        std::vector<glm::vec3> v_pos;

        for(int i = 0; i < number_vertices*no_edges_avg; i++)
        {
            int p1 = rand_vertex(gen);
            int p2 = rand_vertex(gen);
            DirectedEdge fwd{p1,p2};
            DirectedEdge bkw{p2,p1};

            gr.AddEdge(fwd);
            gr.AddEdge(bkw);                
        }
        for(int i = 0; i < number_vertices; i++)
        {
            v_pos.push_back( glm::vec3(x_dis(gen), y_dis(gen), z_dis(gen) ) );
        }

        return std::make_shared<Graph3D>(gr,v_pos);
    }*/
};