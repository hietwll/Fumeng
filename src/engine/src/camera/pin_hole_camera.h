#ifndef FUMENG_PIN_HOLE_CAMERA_H
#define FUMENG_PIN_HOLE_CAMERA_H

#include <engine/core/camera.h>
#include <engine/core/transform.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <glm/gtc/matrix_transform.hpp>

FM_ENGINE_BEGIN

class PinHoleCamera : public Camera
{
protected:
    vec3 m_pos; // world space
    vec3 m_look_at; // world space
    vec3 m_up; // world space
    real m_fov; // Degree
    real m_aspect = 1.0_r;
    real m_focal_distance;
    real m_film_width = 1.0_r;
    real m_film_height = 1.0_r;
    Transform m_camera_to_world;
public:
    PinHoleCamera(const PinHoleCameraConfig& config);
    Ray SampleRay(const vec2& ndc_pos, const vec2& lens_sample) const override;
    Transform LookAt(vec3 eye, vec3 dst, vec3 up);
};

FM_ENGINE_END

#endif