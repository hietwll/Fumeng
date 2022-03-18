#ifndef FM_ENGINE_CREATE_CAMERA_H
#define FM_ENGINE_CREATE_CAMERA_H

#include <engine/core/camera.h>

FM_ENGINE_BEGIN

SP<Camera> CreatePinPoleCamera(const PinHoleCameraConfig& config);

SP<Camera> CreateThinLensCamera(const ThinLensCameraConfig& config);

FM_ENGINE_END

#endif