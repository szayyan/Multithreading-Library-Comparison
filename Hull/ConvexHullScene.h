#include "Scene.h"
#include "ParallelData.h"
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <stack>
#include "QuickHull.h"
#include "ThreadColouring.h"
#include "ConvexHullParallel.h"

class ConvexHullScene: public IScene
{
    /* */
public:
    ConvexHullScene(ParallelData _data);
    ~ConvexHullScene();
    void GeneratePointsAndHull();
    void DrawHull(const QuickHull& hull, glm::vec3 colour);
    void InitOpenGL(float ratio);
    void UpdateFrame();
private:
    /* Hull data */
    std::vector<std::vector<QuickHull>> hulls_to_draw;
    std::vector<std::vector<glm::vec3>> hull_colours;
    std::vector<glm::vec2> point_cloud;
    std::vector<glm::vec2> line_positions;

    /* OpenGL variables */
    GLuint shader_program;
    GLuint point_vao;
    GLuint point_colour_attrib;
    GLuint point_vertex_attrib;
    GLuint point_index_attrib;
    GLuint point_position_attrib;
    GLuint line_vao;
    GLuint line_position_buffer;
    GLuint line_colour_attrib;
    GLuint faces_vao;
    GLuint face_colour_attrib;
    GLuint model_id;
    
    glm::vec3 point_colours;
    std::vector<glm::vec3> line_colours;
    // points generated within this volume
    glm::vec2 volume;

    // model data
    int index_count;
    int vert_count;

    // program data
    ParallelData data;

    /* Recreation variables - control how often a new point set is formed */
    int iter;
    int layer;
    double time_between_layer;
    double last_time_drawn;
};

