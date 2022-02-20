#ifndef FM_ENGINE_AGGREGATE_H
#define FM_ENGINE_AGGREGATE_H

#include <engine/common.h>
#include <engine/core/render_object.h>
#include <vector>

FM_ENGINE_BEGIN

class Aggregate
{
protected:
    std::vector<SP<const RenderObject>> m_objects;

public:
    Aggregate(const std::vector<SP<const RenderObject>>& objects) : m_objects(objects) {};

    virtual ~Aggregate() = default;

    /**
     * Test if the ray intersect with this geometry
     * Ray is in world space
     */
    virtual bool IsIntersect(const Ray &r) const = 0;

    /**
     * Get the HitPoint information if there is a intersection
     * Both Ray and HitPoint should be in world space
     */
    virtual bool GetIntersect(const Ray &r, HitPoint *hit_point) const = 0;

    virtual void ConstructAreaLight(std::vector<SP<const Light>>& lights) const
    {
        for(auto& obj : m_objects) {
            obj.get()->ConstructAreaLight(lights);
        }
    };
};

FM_ENGINE_END

#endif