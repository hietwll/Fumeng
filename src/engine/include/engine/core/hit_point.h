#ifndef FM_ENGINE_HIT_POINT_H
#define FM_ENGINE_HIT_POINT_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class HitPoint
{
public:
    SP<BSDF> bsdf = nullptr;
    vec3 ng;
    vec3 ns;
    vec3 ss;
};

FM_ENGINE_END

#endif