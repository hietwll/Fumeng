#ifndef FM_ENGINE_CAMERA_H
#define FM_ENGINE_CAMERA_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Material
{
public:
    virtual ~Material() = default;
    virtual void ComputeBSDF(HitPoint& hit_point) const = 0;
};

FM_ENGINE_END

#endif