#include "Graph.h"
#include <iostream>
#include <random>
#include <algorithm>

Graph::Graph( int number_of_verticies )
{
    adjacency_list.resize(number_of_verticies);
}

void Graph::AddEdge( DirectedEdge edge )
{
    adjacency_list[edge.start].push_back( edge.end );
}

int Graph::Size()
{
    return adjacency_list.size();
}

void Graph::OutputGraph()
{
    /* Prints formatted graph */
    for( uint i = 0; i < adjacency_list.size(); i++ )
    {
        std::cout << i;
        for( uint j = 0; j < adjacency_list[i].size(); j++)
        {
            std::cout << " " << adjacency_list[i][j];
        }
        std::cout << std::endl;
    }
}

Graph Graph::GenerateRandomGraph(int no_nodes, int max_edges, float p)
{
    /*  Generates a random graph. Edges are calculated using a binomial distribution with
        n = max_edges and p = p */

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::binomial_distribution<int> con_prob( max_edges , p );
    Graph gr = Graph(no_nodes);

    std::cout << "Beginning Random Graph Generation..." << std::endl;

    // iterates through all nodes
    for(int i = 1; i < no_nodes; i++)
    {
        // calculates degree of this vertex
        int no_edges = con_prob(gen);
        // choose edge from already existing edge
        std::uniform_int_distribution<int> edge_selector(0,i-1);
        // vector of chosen edges to prevent same edge selected twice
        std::vector<int> chosen_edges; 

        for(int j = 0; j < no_edges; j++)
        {
            int random_neighbour = edge_selector(gen);
            // if edge does not already exist then add edge to graph
            if( std::find(chosen_edges.begin(),chosen_edges.end(),random_neighbour) ==
                chosen_edges.end() )
            {
                chosen_edges.push_back( random_neighbour );
                gr.AddEdge( DirectedEdge{ i, random_neighbour } );
                gr.AddEdge( DirectedEdge{ random_neighbour, i });
            }
        }
    }

    std::cout << "Graph Generation Complete " << std::endl << std::endl;

    return gr;
}