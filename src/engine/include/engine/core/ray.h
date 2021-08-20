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

    Ray() : Ray(vec3(), vec3(1.0_r, 0.0_r, 0.0_r)){};
    Ray(const vec3& o, const vec3& d, real tmax = REAL_MAX) : ori(o), dir(d), t_max(tmax){};
    vec3 operator()(real time) const { return std::move(ori + dir * time); };

    ~Ray() = default;
};

FM_ENGINE_END

#endif