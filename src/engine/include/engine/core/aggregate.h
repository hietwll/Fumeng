#ifndef FM_ENGINE_AGGREGATE_H
#define FM_ENGINE_AGGREGATE_H

#include <engine/common.h>

#include <vector>

FM_ENGINE_BEGIN

class Aggregate
{
public:
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
};

FM_ENGINE_END

#endif