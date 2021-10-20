#include <engine/core/scene.h>

#include <iostream>

FM_ENGINE_BEGIN

class SimpleScene : public Scene
{
public:
    SimpleScene(SP<const Camera>& camera_, SP<const Aggregate>& aggregate_)
    : Scene(camera_, aggregate_)
    {
    }

};

SP<Scene> CreateSimpleScene(SP<const Camera>& camera, SP<const Aggregate>& aggregate)
{
    return MakeSP<SimpleScene>(camera, aggregate);
}

FM_ENGINE_END