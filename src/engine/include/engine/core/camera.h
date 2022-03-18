#ifndef FM_ENGINE_CAMERA_H
#define FM_ENGINE_CAMERA_H

#include <engine/common.h>
#include <engine/core/config.h>

FM_ENGINE_BEGIN

class PinHoleCameraConfig : public Config
{
public:
    vec3 pos = black; // world space
    vec3 look_at = black; // world space
    vec3 up = blue; // world space
    real fov = 60.0_r; // Degree
    real aspect = 1.0_r;
    real focal_distance = 1.0_r;

    void Load(const nlohmann::json &j) override
    {
        Config::Load(j);
        FM_LOAD_IMPL(j, pos);
        FM_LOAD_IMPL(j, look_at);
        FM_LOAD_IMPL(j, up);
        FM_LOAD_IMPL(j, fov);
        FM_LOAD_IMPL(j, aspect);
        FM_LOAD_IMPL(j, focal_distance);
    }
};

class ThinLensCameraConfig : public PinHoleCameraConfig
{
public:
    real lens_radius;

    void Load(const nlohmann::json &j) override
    {
        PinHoleCameraConfig::Load(j);
        FM_LOAD_IMPL(j, lens_radius);   
    }    
};

class Camera
{
public:
    virtual ~Camera() = default;
    virtual Ray SampleRay(const vec2& ndc_pos, const vec2& lens_sample) const = 0;
};

FM_ENGINE_END

#endif