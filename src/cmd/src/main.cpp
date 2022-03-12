#include <engine/create/scene_loader.h>

using namespace fumeng::engine;

void excute(const std::string& scene_str)
{
    std::filesystem::path scene_path(scene_str);
    SceneLoader scene_loader;
    scene_loader.Load(absolute(scene_path));
    scene_loader.DrawFrame();
}

int main(int argc, char *argv[])
{
    try {
        if (argc < 1) {
            spdlog::error("Please input the scene file.");
            return -1;
        } else {
            std::string scene_str{argv[1]};
            if (scene_str.find(".json")==std::string::npos) {
                spdlog::error("Please specify the correct scene file.");
            } else {
                excute(scene_str);
                return 0;
            }
        }
    }
    catch (...) {
        spdlog::error("Unexpected error.");
    }
    return -1;
}