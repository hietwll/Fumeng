#include <engine/core/scene.h>

#include <iostream>

FM_ENGINE_BEGIN

class SimpleScene : public Scene
{
public:
    SimpleScene(SP<Camera>& camera_, SP<Camera>& aggregate_)
    : camera(camera_), aggregate(aggregate_)
    {
        aggregate->ConstructAreaLight(lights);
    }

};

FM_ENGINE_END