#ifndef FM_ENGINE_LIGHT_H
#define FM_ENGINE_LIGHT_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Light
{
public:

    virtual ~Light() = default;

    /**
     * sample light wi at ref
     */
    virtual vec3 Sample(const HitPoint& hit_point, const vec2& sample, vec3 *wi, real *pdf) const = 0;

    /**
     * is this a delta light (delta position or delta direction)
     */
    virtual bool IsDelta() const = 0;
};

class AreaLight : public Light
{
private:
    const Geometry* geometry = nullptr;
    vec3 radiance;

public:
    AreaLight(const Geometry* geometry_, const vec3& radiance_)
    : geometry(geometry_), radiance(radiance_)
    {

    }

    // todo: add sample method for area light
    vec3 Sample(const HitPoint& hit_point, const vec2& sample, vec3 *wi, real *pdf) const override
    {
        return  {};
    }

    bool IsDelta() const override
    {
        return false;
    }
};


FM_ENGINE_END