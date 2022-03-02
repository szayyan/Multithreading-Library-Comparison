#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <iostream>

struct Edge
{
    /* Simple edge data structure */

    glm::vec2 start;
    glm::vec2 direction;

    Edge(glm::vec2 s, glm::vec2 e):
    start(s),
    direction(e-s)
    {     }

    float GetPointCCW(glm::vec2 point)
    {
        /* return >0 if ccw <0 if cw and 0 if colinear */
        float d = ( point.y - start.y ) * direction.x -
                  ( point.x - start.x ) * direction.y;
        return d;
    }

    float GetDistanceToPoint2(glm::vec2 point)
    {
        glm::vec2 point_to_line = point - start;
        float dot = glm::dot( direction , point_to_line );
        return glm::length2(point_to_line) - ((dot*dot)/glm::length2(direction));
    }

    bool ContainsPoint(glm::vec2 point)
    {
        // checks if either point is within the edge
        return start == point || start + direction == point;
    }

    void Reverse()
    {
        start = start + direction;
        direction *= -1;
    }
    
    std::string CastToString()
    {
        return "start " + glm::to_string(start) + " end " + glm::to_string(start+direction);
    }
};
