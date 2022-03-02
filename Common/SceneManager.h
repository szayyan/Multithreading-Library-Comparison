#include "Scene.h"
#include <memory>

class SceneManager
{
    public:
        SceneManager( std::unique_ptr<IScene>& _scene );
        void Run();
        ~SceneManager();
    private:
        std::unique_ptr<IScene>& scene;

};
