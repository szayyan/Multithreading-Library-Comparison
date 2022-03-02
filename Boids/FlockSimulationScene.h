#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <memory>
#include "BoidManager.h"
#include "Scene.h"
#include "ParallelData.h"

class FlockSimulationScene : public IScene
{
    /* Renders the flock simulation using OpenGL */

    public:
        FlockSimulationScene( ParallelData _data );
        void InitOpenGL(float ratio);
        void UpdateFrame();
        ~FlockSimulationScene();
    private:
        // OpenGL Variables
        GLuint boid_vao;
        GLuint boid_vertex_buffer;
        GLuint boid_index_buffer;
        GLuint obstacle_vao;
        GLuint boid_colour_attribute;
        GLuint obstacle_colour_attribute;
        GLuint boid_transformation_buffer;
        GLuint obstacle_position_buffer;
        GLuint shader_program;
        GLuint normal_buffer;
        int index_count;
        int vert_count;
        
        // simulation data
        int boid_count;
        int iteration;

        ParallelData data;

        std::vector<glm::mat4> boid_matricies;
        std::vector<glm::vec3> boid_colours;
        std::unique_ptr<BoidManager> boid_manager;
        // per frame iteration function
        std::function<void()> update_func;
};