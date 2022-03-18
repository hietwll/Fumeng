#include "pin_hole_camera.h"

FM_ENGINE_BEGIN

PinHoleCamera::PinHoleCamera(const PinHoleCameraConfig& config) :
m_pos(config.pos),
m_look_at(config.look_at),
m_up(glm::normalize(config.up)),
m_focal_distance(config.focal_distance),
m_fov(config.fov),
m_aspect(config.aspect)
{
    m_camera_to_world = Transform(glm::lookAt(m_pos, m_look_at, m_up)).InvTransform(); //LookAt(m_pos, m_look_at, m_up);
    m_film_height = 2.0_r * m_focal_distance * std::tan(DegToRad(m_fov) / 2.0_r);
    m_film_width = m_aspect * m_film_height;
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
 * Sample the camera and return a ray in world space
 */
Ray PinHoleCamera::SampleRay(const vec2 &ndc_pos, const vec2& lens_sample) const
{
    const vec3 film_pos_loc = vec3((ndc_pos.x - 0.5_r) * m_film_width, (ndc_pos.y - 0.5_r) * m_film_height, m_focal_distance);
    const vec3 film_pos = m_camera_to_world.ApplyToPoint(film_pos_loc);
    Ray ray(m_pos, glm::normalize(film_pos - m_pos));
    return ray;
}

SP<Camera> CreatePinPoleCamera(const PinHoleCameraConfig& config)
{
    return MakeSP<PinHoleCamera>(config);
}

FM_ENGINE_END