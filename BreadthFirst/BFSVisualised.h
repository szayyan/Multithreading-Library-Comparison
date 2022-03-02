#include "Graph3D.h"
#include <memory>
#include <map>
#include "VertexBag.h"
#include "ParallelData.h"
#include <thread>
#include <mutex>
#include "ThreadPool.h"

class BFSVisualised
{
// similar class to BFS except it handles colours and iterate must be called explicitly
// see BFS.h/BFS.cpp for more in depth explanation of functions/vars.
public:
    BFSVisualised(  std::shared_ptr<Graph> _graph, int min_par_depth, ParallelData _data,
                    std::vector<glm::vec3>& node_colours, const std::vector<glm::vec3>& thread_colours,
                    std::vector<glm::vec3>& line_colours, std::vector<DirectedEdge>& edges_drawn );
    bool Iterate();
    ~BFSVisualised();
private:
    bool IterateOMP();
    bool IterateSTD();
    void ProccessLevelSTD( VertexBag*& bag_1, VertexBag*& bag_2, int depth );
    void ProccessLevelOMP( VertexBag*& bag_1, VertexBag*& bag_2 , int depth);
    VertexBag* start_bag;
    std::shared_ptr<Graph> graph;
    std::vector<std::thread::id> thread_ids;
    int granularity;
    int max_depth;
    std::vector<short> visited;
    std::vector<glm::vec3>& node_colours;
    const std::vector<glm::vec3>& thread_colours;
    std::vector<glm::vec3>& line_colours;
    std::vector<DirectedEdge>& edges;
    std::mutex id_mutex;
    ThreadPool thread_pool;
    int is_omp;
};

