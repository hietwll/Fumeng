#ifndef FM_ENGINE_CAMERA_H
#define FM_ENGINE_CAMERA_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Camera
{
public:
    virtual ~Camera() = default;
    virtual Ray GetRandomRay()
};

FM_ENGINE_END

#endif