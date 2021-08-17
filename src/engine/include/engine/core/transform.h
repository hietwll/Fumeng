#ifndef FM_ENGINE_TRANSFORM_H
#define FM_ENGINE_TRANSFORM_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Transform
{
private:
    mat4 matrix_, inv_;
public:
    Transform() = default;
    Transform(const mat4& mat);
    Transform(const mat4& mat, const mat4& inv_mat);
    Transform(const vec3& translation, const vec3& rotation, const vec3& scale = vec3(1.0, 1.0, 1.0));

    Transform InvTransform() const;
    vec3 ApplyToNormal(const vec3& norm) const;
    vec3 ApplyToVec3(const vec3& vec) const;
};

FM_ENGINE_END

#endif