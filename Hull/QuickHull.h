#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <stack>
#include "Edge.h"

class QuickHull
{
    /*  Applies the quickhull algorithm on the set of points in the constructor 
        then results are stored in the edges variable */
public:
    QuickHull();
    QuickHull(const std::vector<glm::vec2>& points);//, std::vector<glm::vec3>& point_colours);
    std::vector<Edge> edges;
    static QuickHull MergeHulls( const QuickHull& hull1, const QuickHull& hull2 );

private:
    std::vector<glm::vec2> point_cloud;
    std::pair<int,int> FindExtremePoints(); 
    void Iterate(std::vector<glm::vec2> points, Edge edge);
};