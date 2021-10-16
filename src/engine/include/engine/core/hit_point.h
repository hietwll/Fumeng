#ifndef FM_ENGINE_HIT_POINT_H
#define FM_ENGINE_HIT_POINT_H

#include <engine/common.h>
#include <engine/core/ray.h>


FM_ENGINE_BEGIN

class HitPoint
{
public:
    SP<const BSDF> bsdf = nullptr;
    const Material* material = nullptr;
    const RenderObject* object = nullptr;

    vec3 pos; // hit point position, world space
    vec3 ng; // geometry normal, world space
    vec3 ns; // shading normal, world space
    vec3 ss; // primary tangent, world space
    vec3 wo_r_w; // reverse of incident direction in world space
    vec2 uv; // uv coordinate

    real t = REAL_MAX; // t for current hitting ray

    Ray GenRay(const vec3& dir) const
    {
        vec3 ori;
        // todo: when object is quite large, eps should also be large
        if (glm::dot(dir, ng) > 0) {
            ori = pos + ng * eps;
        } else {
            ori = pos - ng * eps;
        }

        return {ori, dir};
    }
};


FM_ENGINE_END

#endif