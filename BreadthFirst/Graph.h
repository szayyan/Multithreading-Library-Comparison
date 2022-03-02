#pragma once
#include <vector>
#include <glm/glm.hpp>

struct DirectedEdge
{
    int start;
    int end;

    // bi-directional equivalence
    bool operator==(const DirectedEdge& other_edge) const
    {
        return  (other_edge.end == start && other_edge.start == end) ||
                (other_edge.start == start && other_edge.end == end);
    }
};

class Graph
{
public:
    std::vector<std::vector<int>> adjacency_list;
    
    Graph( int number_of_verticies );
    void AddEdge( DirectedEdge edge );
    int Size();
    void OutputGraph();

    static Graph GenerateRandomGraph(int no_nodes, int max_edges, float p);
};


