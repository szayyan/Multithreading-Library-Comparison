#define GLM_ENABLE_EXPERIMENTAL
#include "Boid.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream> 
#include <glm/gtx/string_cast.hpp>
#include "Quat.h"

Boid::Boid( glm::vec3 _position, glm::vec3 _velocity, int _id ):
            position(_position), 
            velocity(_velocity),
            id(_id),
            steering(0.0f,0.0f,0.0f)
{}

glm::mat4 Boid::GetMatrix()
{
    /*  calculates the transformation matrix given the position and velocity
        required in order to render the boid in 3D space  quatLookAt is non backward compatible may need
        to implement if */

    glm::mat4 matrix(1.0f);

    // apply translation
    matrix = glm::translate(matrix,position);
    // apply rotation
    // boids 'look' in the direction of their velocity 
    // must be further rotated along the y axis by -pi/2 in order to map the +z to the +y axis
    // this is done so the model points with the 'nose' of the plane forward
    matrix *= glm::toMat4(Quat::LookAt( glm::normalize(velocity) , glm::vec3(0,1,0))*glm::angleAxis( -1.57079632679f, glm::vec3(1,0,0) ));

    return matrix;
}

void Boid::ComputeBoundarySteer(float max_dist)
{
    /* Computes the steer force is the boid has gone out of bounds */

    // spherical volume bound from origin
    if( glm::length(position) > max_dist )
    {
        // steer towards (0,0,0)
        glm::vec3 desired_velocity = -position;
        ClampVector( desired_velocity, BoidAttributesShared::max_centre_seek_force);
        steering += (desired_velocity - velocity);
    }
}  

// compute separation alignment and cohesion for a given boid
void Boid::ComputeSAC()
{
    glm::vec3 separation(0.0f);
    glm::vec3 alignment(0.0f);
    glm::vec3 cohesion(0.0f);

    float separation_neighbours = 0;
    float alignment_neighbours = 0;
    float cohesion_neighbours = 0;

    // iterate through all neighbours in the boids volume
    for(size_t i = 0; i < neighbours.size(); i++)
    {
        glm::vec3 offset = neighbours[i]->position - position;
        float dist = glm::length(offset);
        float cosAngleBetweenBoids = glm::dot( glm::normalize(velocity), offset / dist );
        
        // -- separation
        if( dist < BoidAttributesShared::separation_radius &&
            cosAngleBetweenBoids > BoidAttributesShared::separation_angle)
        { 
            separation_neighbours++;
            float offset_sqr = glm::dot(offset,offset);
            separation += (offset / -offset_sqr);
        } 
        // -- alignment
        if( dist < BoidAttributesShared::alignment_radius && 
            cosAngleBetweenBoids > BoidAttributesShared::alignment_angle)
        {
            alignment_neighbours++;
            alignment += glm::normalize(neighbours[i]->velocity);
        }  
        // -- cohesion
        if( dist < BoidAttributesShared::cohesion_radius &&
            cosAngleBetweenBoids > BoidAttributesShared::cohesion_angle )
        { 
            cohesion_neighbours++;
            cohesion += neighbours[i]->position;
        } 
    }

    // prevent divide by 0
    if( separation_neighbours > 0 )
        separation = glm::normalize(separation / separation_neighbours);

    if( alignment_neighbours > 0 )
        alignment = glm::normalize((alignment/(float)alignment_neighbours));// - glm::normalize(velocity));

    if( cohesion_neighbours > 0)
        cohesion = glm::normalize( (cohesion/cohesion_neighbours)- position);
    
    // apply individual components to steering weighted by their attributes
    steering += separation * BoidAttributesShared::separation_weight +
                alignment * BoidAttributesShared::alignment_weight +
                cohesion * BoidAttributesShared::cohesion_weight;
}
   

void Boid::ApplySteering(float delta_time) 
{
    /* Update boids position by applying steering forces */

    ClampVector(steering, BoidAttributesShared::max_steering_force);
    velocity += steering;
    ClampVector(velocity, BoidAttributesShared::max_speed);
    // delta_time ensures boids travel at roughly the same speed regardless
    // of frame rate
    position += (velocity * delta_time);
    steering = glm::vec3(0.0,0.0,0.0);
} 

void Boid::ClampVector(glm::vec3& vec, float max_size)
{
    /* clamp the magnitude of a given vector to max_size */
    
    if( glm::length(vec) > max_size )
    {
        vec = glm::normalize(vec) * max_size;
    }
}