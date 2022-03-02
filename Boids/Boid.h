#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace BoidAttributesShared
{
    /* shared variables that control the movement and behaviour of all boids in the scene */

    // max steering force that can be applied per iteration
    const float max_steering_force = 0.005f;
    // max speed that a boid can travel per iteration
    const float max_speed = 0.2f;
    // max magnitude that boids will return to the centre with
    const float max_centre_seek_force = 0.2f;

    // radius of sphere describing the area each boid will check for the other boids
    const float radius = 40.0f;

    // seperation will only be applied if the boid is within the given radius
    // the angle between a given boid and its neighbour must be greater than the separation_angle
    // for it to be detected to account for 'line of sight'.
    // weight refers to the relative strength of the separation force with respect to the other forces
    const float separation_radius =  20.0f;
    const float separation_angle  = -0.7f;
    const float separation_weight =  0.3f;

    // refer to above for variable meaning. In this case they refer to alignment however.
    const float alignment_radius = 25.0f;
    const float alignment_angle  = 0.4f;
    const float alignment_weight = 0.3f;

    // refer to above for variable meaning. In this case they refer to cohesion however.
    const float cohesion_radius = 30.0f;
    const float cohesion_angle  = -0.15f;
    const float cohesion_weight = 0.35f;
};

class Boid
{
    /* container for each boid instance in the simulation */
public:
    Boid(glm::vec3 _position, glm::vec3 _velocity, int _id );

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 steering;
    std::vector<Boid*> neighbours;
    int id;
    
    glm::mat4 GetMatrix();
    void ComputeSAC();
    void ComputeBoundarySteer(float max_dist);
    void ApplySteering( float delta_time );
    void ClampVector(glm::vec3& vec, float max_size);
};