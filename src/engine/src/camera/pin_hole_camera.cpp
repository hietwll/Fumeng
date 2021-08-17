#include <engine/core/camera.h>
#include <engine/core/transform.h>
#include <engine/core/ray.h>
#include <glm/gtc/matrix_transform.hpp>

FM_ENGINE_BEGIN

class PinHoleCamera : public Camera
{
private:
    vec3 pos_; // world space
    vec3 look_at_; // world space
    vec3 up_; // world space
    real fov_;
    real aspect_ = 1.0;
    real focal_distance_;
    real film_width_ = 1.0;
    real film_height_ = 1.0;
    Transform camera_to_word;
public:
    PinHoleCamera(const vec3& pos, const vec3& look_at, const vec3& up, real focal_distance, real fov = 60.0, real aspect = 1.0);
    Ray SampleRay(const vec2& ndc_pos) const override;
};

PinHoleCamera::PinHoleCamera(const vec3 &pos, const vec3 &look_at, const vec3 &up, real focal_distance, real fov,
                             real aspect)
                             : pos_(pos), look_at_(look_at), up_(up), focal_distance_(focal_distance), fov_(fov), aspect_(aspect)
{
    camera_to_word = Transform(glm::lookAt(pos_, look_at_, up_));
    film_height_ = 2.0 * focal_distance_ * std::tan(fov_ / 2.0);
    film_width_ = aspect_ * film_height_;
}

Ray PinHoleCamera::SampleRay(const vec2 &ndc_pos) const
{
    const vec3 film_pos = camera_to_word.ApplyToVec3(
            vec3((ndc_pos.x - 0.5) * film_width_, (ndc_pos.y - 0.5) * film_height_, focal_distance_));
    Ray ray(pos_, glm::normalize(film_pos - pos_));
    return ray;
}

FM_ENGINE_END