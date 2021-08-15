#ifndef FM_ENGINE_CAMERA_H
#define FM_ENGINE_CAMERA_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Camera
{
public:
    virtual ~Camera() = default;
    virtual Ray SampleRay(const vec2& ndc_pos) const = 0;
};

FM_ENGINE_END

#endif