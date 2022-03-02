#include "Scene.h"
#include "ParallelData.h"
#include "Graph3D.h"
#include <vector>
#include <GL/glew.h>
#include <memory>
#include "ThreadColouring.h"
#include "BFSVisualised.h"
#include <functional>

class BFSScene : public IScene
{
    public:
        BFSScene(ParallelData _data);
        void InitOpenGL( float ratio );
        void UpdateFrame();
        void UpdateEdgeData();
        ~BFSScene();
    private:
        // information container for program - number of threads, objects, etc.
        ParallelData data;

        // OpenGL variables - required for rendering
        GLuint VAO;
        GLuint lineVAO;
        GLuint cube_color_attribute;
        GLuint line_color_attribute;
        GLuint lineVBO;
        GLuint VBO;
        GLuint node_position_buffer;
        GLuint shader_program;
        GLuint elementbuffer;
        int index_count;
        
        // time between each graph iteration
        double time_between_updates;
        // last time the graph iterated
        double last_time_updated;

        // Used for visualisation 
        ThreadColouring thread_colours;
        std::vector<glm::vec3> line_colours;
        std::vector<glm::vec3> cube_colours;
        std::vector<glm::vec3> line_verts;
        std::vector<DirectedEdge> edges_drawn;

        // 3D graph - and class that will search it
        std::shared_ptr<Graph3D> spatial_graph;
        std::unique_ptr<BFSVisualised> bfs;
};
