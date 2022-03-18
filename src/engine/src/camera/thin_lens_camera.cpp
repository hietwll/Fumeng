#include <engine/core/sampler.h>
#include <engine/core/utils.h>
#include "pin_hole_camera.h"


FM_ENGINE_BEGIN

class ThinLensCamera : public PinHoleCamera
{
private:
    real m_radius;

public:
    ThinLensCamera(const ThinLensCameraConfig& config) :
    PinHoleCamera(config),
    m_radius(config.lens_radius)
    {
    }

    /*
    * Sample the camera and return a ray in world space
    */
    Ray SampleRay(const vec2 &ndc_pos, const vec2& lens_sample) const override
    {
        const vec3 film_pos_loc = vec3((ndc_pos.x - 0.5_r) * m_film_width, (ndc_pos.y - 0.5_r) * m_film_height, m_focal_distance);
        Ray ray;
        ray.ori = {m_radius * lens_sample.x, m_radius * lens_sample.y, 0.0_r};
        ray.dir = glm::normalize(film_pos_loc - ray.ori);
        return m_camera_to_world.ApplyToRay(ray);
    }
};

SP<Camera> CreateThinLensCamera(const ThinLensCameraConfig& config)
{
    return MakeSP<ThinLensCamera>(config);
}

FM_ENGINE_END