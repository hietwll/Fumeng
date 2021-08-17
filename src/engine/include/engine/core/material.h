#ifndef FM_ENGINE_MATERIAl_H
#define FM_ENGINE_MATERIAl_H

#include <engine/common.h>
#include <engine/core/bsdf.h>

FM_ENGINE_BEGIN

class Material
{
public:
    virtual ~Material() = default;
    virtual void ComputeBSDF(HitPoint& hit_point) const = 0;
};

FM_ENGINE_END

#endif