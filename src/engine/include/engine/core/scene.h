#ifndef FM_ENGINE_SCENE_H
#define FM_ENGINE_SCENE_H

#include <engine/common.h>
#include <engine/core/aggregate.h>
#include <vector>

FM_ENGINE_BEGIN

class Scene
{
protected:
    SP<const Camera> camera;
    SP<const Aggregate> aggregate;
    std::vector<SP<const Light>> lights;

public:
    Scene(SP<const Camera>& camera_, SP<const Aggregate>& aggregate_)
    : camera(camera_), aggregate(aggregate_)
    {
        aggregate->ConstructAreaLight(lights);
    }

    SP<const Camera> GetCamera()
    {
        return camera;
    }

    bool IsIntersect(const Ray &r) const
    {
        return aggregate->IsIntersect(r);
    }

    std::vector<const Light*> GetLights() const
    {
        std::vector<const Light*> res;
        for(auto& light : lights) {
            res.push_back(light.get());
        }
        return res;
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const
    {
        return aggregate->GetIntersect(r, hit_point);
    }

    virtual ~Scene() = default;
};

FM_ENGINE_END

#endif