#ifndef FM_ENGINE_RAY_H
#define FM_ENGINE_RAY_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Ray
{
public:

    vec3 ori;
    vec3 dir; // should be normalized
    real t_max;
    real t_min;

    Ray() : Ray(vec3(), vec3(1.0_r, 0.0_r, 0.0_r))
    {

    };

    Ray(const vec3& o, const vec3& d, real tmin = 0.0_r, real tmax = REAL_MAX)
    : ori(o), dir(d), t_min(tmin), t_max(tmax)
    {

    };

    vec3 operator()(real time) const
    {
        return ori + dir * time;
    };

    bool Contains(const real& t) const
    {
        return t >= t_min && t <= t_max;
    }

    ~Ray() = default;
};

FM_ENGINE_END

#endif