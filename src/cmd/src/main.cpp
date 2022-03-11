#include <engine/create/scene_loader.h>

using namespace fumeng::engine;

int main()
{
    SceneLoader scene_loader;
    std::string scene_path = "helmet.json";
    scene_loader.Load(scene_path);
    scene_loader.DrawFrame();
    return 0;
}