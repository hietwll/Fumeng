#ifndef FM_ENGINE_SCENE_H
#define FM_ENGINE_SCENE_H

#include <engine/common.h>
#include <engine/core/aggregate.h>
#include <vector>

FM_ENGINE_BEGIN

class Scene
{
private:
    SP<const Camera> camera;
    SP<const Aggregate> aggregate;
    std::vector<SP<const Light>> lights;

public:
    Scene(SP<Camera>& camera_, SP<Aggregate>& aggregate_)
    : camera(camera_), aggregate(aggregate_)
    {
        aggregate->ConstructAreaLight(lights);
    }

    SP<const Camera> GetCamera()
    {
        return camera;
    }

    virtual ~Scene() = default;
};

FM_ENGINE_END

#endif