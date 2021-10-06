#include <engine/core/transform.h>
#include <glm/gtx/transform.hpp>
#include <engine/core/ray.h>
#include <engine/core/hit_point.h>

FM_ENGINE_BEGIN

Transform::Transform(const mat4 &mat) : matrix_(mat), inv_(glm::inverse(mat))
{
}

Transform::Transform(const mat4 &mat, const mat4 &inv_mat) : matrix_(mat), inv_(inv_mat)
{
}

Transform::Transform(const vec3 &translation, const vec3 &rotation, const vec3 &scale)
{
    mat4 rot = glm::rotate(rotation.z, vec3(0.0_r, 0.0_r, 1.0_r)) *
            glm::rotate(rotation.y, vec3(0.0_r, 1.0_r, 0.0_r)) *
            glm::rotate(rotation.x, vec3(1.0_r, 0.0_r, 0.0_r));
    matrix_ = glm::translate(mat4(1.0_r), translation) * rot *
            glm::scale(mat4(1.0_r), scale);
    inv_ = glm::inverse(matrix_);
}

Transform Transform::InvTransform() const
{
    return {inv_, matrix_};
}

vec3 Transform::ApplyToVec3(const vec3 &vec) const
{
    const auto v4 = matrix_ * vec4(vec, 0.0_r);
    return {v4.x, v4.y, v4.z};
}

vec3 Transform::ApplyToPoint(const vec3 &vec) const
{
    const auto v4 = matrix_ * vec4(vec, 1.0_r);
    return {v4.x / v4.w, v4.y / v4.w , v4.z / v4.w};
}

Ray Transform::ApplyToRay(const Ray &r) const
{
    const vec3 ori = ApplyToPoint(r.ori);
    const vec3 dir = ApplyToVec3(r.dir);
    return {ori, dir, r.t_min, r.t_max};
}

void Transform::ApplyToHitPoint(HitPoint* hit_point) const
{
    hit_point->pos = ApplyToPoint(hit_point->pos);
    hit_point->ng = ApplyToVec3(hit_point->ng);
    hit_point->ns = ApplyToVec3(hit_point->ns);
    hit_point->ss = ApplyToVec3(hit_point->ss);
}

vec3 Transform::ApplyToNormal(const vec3 &norm) const
{
    const auto v4 = glm::transpose(inv_) * vec4(norm, 1.0_r);
    return glm::normalize(vec3(v4.x, v4.y, v4.z));
}

FM_ENGINE_END