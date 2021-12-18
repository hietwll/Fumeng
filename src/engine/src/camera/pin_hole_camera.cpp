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
    real aspect_ = 1.0_r;
    real focal_distance_;
    real film_width_ = 1.0_r;
    real film_height_ = 1.0_r;
    Transform camera_to_world;
public:
    PinHoleCamera(const vec3& pos, const vec3& look_at, const vec3& up, real focal_distance, real fov = 60.0_r, real aspect = 1.0_r);
    Ray SampleRay(const vec2& ndc_pos) const override;
    Transform LookAt(vec3 eye, vec3 dst, vec3 up);
};

PinHoleCamera::PinHoleCamera(const vec3 &pos, const vec3 &look_at, const vec3 &up, real focal_distance, real fov,
                             real aspect)
                             : pos_(pos), look_at_(glm::normalize(look_at)), up_(glm::normalize(up)),
                             focal_distance_(focal_distance), fov_(fov), aspect_(aspect)
{
    camera_to_world = Transform(glm::lookAt(pos_, look_at_, up_)).InvTransform(); //LookAt(pos_, look_at_, up_);//
    film_height_ = 2.0_r * focal_distance_ * std::tan(fov_ / 2.0_r);
    film_width_ = aspect_ * film_height_;
}

Transform PinHoleCamera::LookAt(vec3 eye, vec3 dst, vec3 up)
{
    auto D = glm::normalize(dst - eye);
    auto R = glm::normalize(glm::cross(up, D));
    auto U = glm::cross(D, R);
    mat4 mat;
    mat[0][0] = R.x;
    mat[1][0] = R.y;
    mat[2][0] = R.z;
    mat[3][0] = 0.0_r;

    mat[0][1] = U.x;
    mat[1][1] = U.y;
    mat[2][1] = U.z;
    mat[3][1] = 0.0_r;

    mat[0][2] = D.x;
    mat[1][2] = D.y;
    mat[2][2] = D.z;
    mat[3][2] = 0.0_r;

    mat[0][3] = eye.x;
    mat[1][3] = eye.y;
    mat[2][3] = eye.z;
    mat[3][3] = 1.0_r;

    return {mat};
}

/*
 * sample the camera and return a ray in world space
 */
Ray PinHoleCamera::SampleRay(const vec2 &ndc_pos) const
{
    const vec3 film_pos_loc = vec3((ndc_pos.x - 0.5_r) * film_width_, (ndc_pos.y - 0.5_r) * film_height_, focal_distance_);
    const vec3 film_pos = camera_to_world.ApplyToPoint(film_pos_loc);
    Ray ray(pos_, glm::normalize(film_pos - pos_));
    return ray;
}

SP<Camera> CreatePinPoleCamera(const vec3& pos, const vec3& look_at, const vec3& up, real focal_distance, real fov = 60.0_r, real aspect = 1.0_r)
{
    return MakeSP<PinHoleCamera>(pos, look_at, up, focal_distance, fov, aspect);
}

FM_ENGINE_END