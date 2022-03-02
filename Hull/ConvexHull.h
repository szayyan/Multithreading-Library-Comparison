#include "ParallelData.h"
#include "Runnable.h"
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <stack>
#include "QuickHull.h"

class ConvexHull: public Runnable
{
    /* Runs Convex Hull algorithm in parllel on randomly generated data */
    public:
        ConvexHull( ParallelData _data );
        ~ConvexHull();
        void Run();
        void OutputStats();
    private:
        glm::vec2 volume;
        std::vector<glm::vec2> point_cloud;
        ParallelData data;
};

