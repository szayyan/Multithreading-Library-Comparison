#include <glm/glm.hpp>
#include <string>
namespace std
{
    template <> 
    struct hash<glm::ivec3>
    {
        // simple custom hashing function for using ivec3s in maps
        std::size_t operator()(const glm::ivec3& k) const
        {
            std::size_t h1 = std::hash<int>()(k.x);
            std::size_t h2 = std::hash<int>()(k.y);
            std::size_t h3 = std::hash<int>()(k.z);
            return ((h1^(h2 << 1) >> 1 ) ^ h3 << 1);
        }
    };
}