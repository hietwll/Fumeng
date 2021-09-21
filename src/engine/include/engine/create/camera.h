#ifndef FM_ENGINE_CREATE_CAMERA_H
#define FM_ENGINE_CREATE_CAMERA_H

#include <engine/core/camera.h>

FM_ENGINE_BEGIN

SP<Camera> CreatePinPoleCamera(const vec3& pos, const vec3& look_at, const vec3& up, real focal_distance, real fov = 60.0_r, real aspect = 1.0_r);

FM_ENGINE_END

#endif