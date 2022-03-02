#include <glm/glm.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "QuickHull.h"
#include "ThreadColouring.h"

class ConvexHullParallel
{
    /* Applies convex hull algorithm in parallel in the map and reduce pattern */
public:
    ConvexHullParallel();
    QuickHull ConvexHullOMP( std::vector<glm::vec2>& point_cloud, float x_spread, int num_threads );
    QuickHull ConvexHullSTDThread(  std::vector<glm::vec2>& point_cloud, float x_spread, int num_threads );
    QuickHull ConvexHullVisual( std::vector<glm::vec2>& point_cloud, float x_spread, int num_threads  );
    QuickHull MapAndReduce( const std::vector<std::vector<glm::vec2>>& hulls );
    std::vector<std::vector<glm::vec2>> PartionPoints(const std::vector<glm::vec2>& point_cloud, float x_max, int number_partitions );
    std::vector<std::vector<QuickHull>> intermediate_hulls;
    std::vector<std::vector<glm::vec3>> intermediate_colours;
private:
    QuickHull MergeHullsParallelVisual( std::vector<QuickHull> hulls, int thread_count );
    QuickHull MergeHullsParallel(std::vector<QuickHull> hulls, int thread_count);
    std::vector<QuickHull> GenerateHullsParallel( std::vector<std::vector<glm::vec2>>& points );
    std::vector<QuickHull> GenerateHullsParallelVisual(std::vector<std::vector<glm::vec2>>& points );
    ThreadColouring pos_colours;
};
