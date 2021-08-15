#include <engine/core/transform.h>
#include <glm/gtx/transform.hpp>

FM_ENGINE_BEGIN

Transform::Transform(const mat4 &mat) : matrix_(mat), inv_(glm::inverse(mat))
{
}

Transform::Transform(const mat4 &mat, const mat4 &inv_mat) : matrix_(mat), inv_(inv_mat)
{
}

Transform::Transform(const vec3 &translation, const vec3 &rotation, const vec3 &scale)
{
    mat4 rot = glm::rotate(rotation.z, vec3(0.0, 0.0, 1.0)) *
            glm::rotate(rotation.y, vec3(0.0, 1.0, 0.0)) *
            glm::rotate(rotation.x, vec3(1.0, 0.0, 0.0));
    matrix_ = glm::translate(mat4(1.0), translation) * rot *
            glm::scale(mat4(1.0), scale);
    inv_ = glm::inverse(matrix_);
}

Transform Transform::InvTransform() const
{
    return std::move(Transform(inv_, matrix_));
}

vec3 Transform::ApplyToVec3(const vec3 &vec) const
{
    const auto v4 = matrix_ * vec4(vec, 1.0);
    return vec3(v4.x, v4.y, v4.z) / v4.w;
}

vec3 Transform::ApplyToNormal(const vec3 &norm) const
{
    const auto v4 = glm::transpose(inv_) * vec4(norm, 1.0);
    return glm::normalize(vec3(v4.x, v4.y, v4.z));
}

FM_ENGINE_END