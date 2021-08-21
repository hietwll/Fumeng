#ifndef FM_ENGINE_HIT_POINT_H
#define FM_ENGINE_HIT_POINT_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class HitPoint
{
public:
    SP<BSDF> bsdf = nullptr;
    vec3 pos; // hit point position, world space
    vec3 ng; // geometry normal, world space
    vec3 ns; // shading normal, world space
    vec3 ss; // primary tangent, world space

    vec2 uv; // uv coordinate
};

FM_ENGINE_END

#endif