#include <engine/core/aggregate.h>
#include <engine/core/ray.h>

FM_ENGINE_BEGIN

class SimpleAggregate : public Aggregate
{
public:
    SimpleAggregate(const std::vector<SP<const RenderObject>>& objects) : Aggregate(objects)
    {
    }

    bool IsIntersect(const Ray &r) const override
    {
        for(const auto& obj : m_objects) {
            if(obj->IsIntersect(r)) {
                return true;
            }
        }
        return false;
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        Ray ray = r;
        bool res = false;
        for(const auto& obj : m_objects) {
            if(obj->GetIntersect(ray, hit_point)) {
                ray.t_max = hit_point->t; // to find the closest hit point
                res = true;
            }
        }
        return res;
    }
};

SP<Aggregate> CreateSimpleAggregate(const std::vector<SP<const RenderObject>>& objects)
{
    return MakeSP<SimpleAggregate>(objects);
}

FM_ENGINE_END