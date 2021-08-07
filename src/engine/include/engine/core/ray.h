#ifndef FM_ENGINE_RAY_H
#define FM_ENGINE_RAY_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Ray
{
public:

    vec3 ori;
    vec3 dir;
    real t_max;

    Ray() : Ray(vec3(), vec3(1.0, 0.0, 0.0)){};
    Ray(const vec3& o, const vec3& d, real tmax = REAL_MAX) : ori(o), dir(d), t_max(tmax){};
    vec3 operator()(real time) { return std::move(ori + dir * time); };

    ~Ray() = default;
};

FM_ENGINE_END

#endif