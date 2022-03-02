#include "QuickHull.h"
#include <iostream>

QuickHull::QuickHull()
{
    // default constructor - used for merging two hulls;
}

QuickHull::QuickHull(const std::vector<glm::vec2>& points):
point_cloud(points)
{    
    if( points.size() < 3 )
    {
        std::cout << "Not Possible to generate hull with less than 3 points" << std::endl;
        return;
    }

    std::pair<int,int> min_max = FindExtremePoints();

    Edge forward_edge( point_cloud[min_max.first], point_cloud[min_max.second] );
    Edge backward_edge( point_cloud[min_max.second], point_cloud[min_max.first] );

    std::vector<glm::vec2> left;
    std::vector<glm::vec2> right;

    for( size_t i = 0; i < point_cloud.size(); i++ )
    {
        if( i == min_max.first || i == min_max.second )
            continue;

        if( forward_edge.GetPointCCW( point_cloud[i] ) > 0.0f )
            left.push_back(point_cloud[i]);
        else
            right.push_back(point_cloud[i]);
    }
    point_cloud.clear();

    Iterate( left, forward_edge );
    Iterate( right, backward_edge );
}

void QuickHull::Iterate(std::vector<glm::vec2> points, Edge edge)
{

    if( points.empty() )
    {
        edges.push_back(edge);
        return;
    }
    float epilson = 0.001f;

    float furthest_dist = 0;
    int furthest_index = 0;
    for(int i = 0; i < points.size(); i++)
    {
        float distance = edge.GetDistanceToPoint2(points[i]);
        if( distance > furthest_dist )
        {
            furthest_dist = distance;
            furthest_index = i;
        }
    }

    Edge partion1( edge.start, points[furthest_index]);
    Edge partion2( points[furthest_index], edge.start + edge.direction);
    
    std::vector<glm::vec2> p1_owned;
    std::vector<glm::vec2> p2_owned;

    for(int i = 0; i < points.size(); i++)
    {
        //if( points[i] == edge.start || points[i] == (edge.start+edge.direction) )
        //    continue;

        if( partion1.GetPointCCW(points[i]) > 0.0f )
            p1_owned.push_back(points[i]);
        else if ( partion2.GetPointCCW(points[i]) > 0.0f )
            p2_owned.push_back(points[i]);
    }

    Iterate(p1_owned,partion1); 
    Iterate(p2_owned,partion2);

}

std::pair<int,int> QuickHull::FindExtremePoints() 
{
    float min_index = 0, max_index = 0;
    float min_x = point_cloud[0].x;
    float max_x = point_cloud[0].x;
 
    for(int i = 1; i < point_cloud.size(); i++)
    {
        if( point_cloud[i].x > max_x )
        {
            max_x = point_cloud[i].x;
            max_index = i;
        }
        else if( point_cloud[i].x < min_x )
        {
            min_x = point_cloud[i].x;
            min_index = i;
        }
    }
    return {min_index,max_index};
}

QuickHull QuickHull::MergeHulls( const QuickHull& left_hull, const QuickHull& right_hull )
{
    /*// all points of left_hull must be left of all all points of right_hull else this will not work*/    

    int lh_size = left_hull.edges.size();
    int rh_size = right_hull.edges.size();

    if( lh_size == 0 )
        return right_hull;
    if( rh_size == 0)
        return left_hull;
    
    int rightmost_point = 0;
    for(size_t i = 1; i < lh_size; i++)
    {
        if( left_hull.edges[i].start.x > left_hull.edges[rightmost_point].start.x )
            rightmost_point = i;
    }

    int leftmost_point = 0;
    for(size_t i = 1; i < rh_size; i++)
    {
        if( right_hull.edges[i].start.x < right_hull.edges[leftmost_point].start.x )
            leftmost_point = i;
    }

    Edge upper_tangent = Edge( left_hull.edges[rightmost_point].start, right_hull.edges[leftmost_point].start );
    int rh_point = leftmost_point;
    int lh_point = rightmost_point;
    bool tangent_found = false;
    while( !tangent_found )
    {
        tangent_found = true;
        int next_ccw_point = (lh_point + 1) % lh_size;
        while( upper_tangent.GetPointCCW(left_hull.edges[next_ccw_point].start) <= 0 )
        {
            lh_point = next_ccw_point;
            upper_tangent = Edge( left_hull.edges[lh_point].start, right_hull.edges[rh_point].start );
            next_ccw_point = (lh_point + 1) % lh_size;
            tangent_found = false;
        }
        int next_cw_point = rh_point == 0 ? rh_size - 1: rh_point - 1;
        while ( upper_tangent.GetPointCCW(right_hull.edges[next_cw_point].start) <= 0 )
        {
            rh_point = next_cw_point;
            upper_tangent = Edge( left_hull.edges[lh_point].start, right_hull.edges[rh_point].start );
            next_cw_point = rh_point == 0 ? rh_size - 1: rh_point - 1;
            tangent_found = false;
        }
    }

    Edge lower_tangent = Edge( left_hull.edges[rightmost_point].start, right_hull.edges[leftmost_point].start );
    int rh_point_l = leftmost_point;
    int lh_point_l = rightmost_point;
    tangent_found = false;

    while( !tangent_found )
    {
        tangent_found = true;
        int next_ccw_point = (rh_point_l + 1) % rh_size;
        while( lower_tangent.GetPointCCW(right_hull.edges[next_ccw_point].start) >= 0 )
        {
            rh_point_l = next_ccw_point;
            lower_tangent = Edge( left_hull.edges[lh_point_l].start, right_hull.edges[rh_point_l].start );
            next_ccw_point = (rh_point_l + 1) % rh_size;
            tangent_found = false;
        }
        int next_cw_point = lh_point_l == 0 ? lh_size - 1: lh_point_l - 1;
        while ( lower_tangent.GetPointCCW(left_hull.edges[next_cw_point].start) >= 0 )
        {
            lh_point_l = next_cw_point;
            lower_tangent = Edge( left_hull.edges[lh_point_l].start, right_hull.edges[rh_point_l].start );
            next_cw_point = lh_point_l == 0 ? lh_size - 1: lh_point_l - 1;
            tangent_found = false;
        }
    }

    QuickHull final_hull;
    final_hull.edges.push_back( lower_tangent );
    int cur_edge = rh_point_l;
    while (cur_edge != rh_point)
    {
        final_hull.edges.push_back(right_hull.edges[cur_edge]);
        cur_edge = (cur_edge + 1) % rh_size;
    }    
    upper_tangent.Reverse();
    final_hull.edges.push_back(upper_tangent);
    cur_edge = lh_point;
    while( cur_edge != lh_point_l )
    {
        final_hull.edges.push_back(left_hull.edges[cur_edge]);
        cur_edge = (cur_edge + 1) % lh_size;
    }

    return final_hull;
}