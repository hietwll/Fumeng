#ifndef FUMENG_MATERIAL_COMMON_H
#define FUMENG_MATERIAL_COMMON_H

#include <engine/common.h>
#include <optional>

FM_ENGINE_BEGIN

namespace mat_func{
    inline real PowerFive(real x)
    {
        const real x2 = x * x;
        return x2 * x2 * x;
    }

    inline vec3 SchlickFresnel(const vec3& F0, real cos_i)
    {
        return F0 + (white - F0) * PowerFive(1.0_r - cos_i);
    }

    inline real SchlickFresnelScalar(const real& F0, real cos_i)
    {
        return F0 + (1.0_r - F0) * PowerFive(1.0_r - cos_i);
    }

    real GTR2Anisotropic(const vec3& wh, real alpha_x, real alpha_y);

    vec3 SampleGTR2(real alpha_x, real alpha_y, const vec2& sample);

    real GTR1GGX(const vec3& wh, real alpha);

    vec3 SampleGTR1(real alpha, const vec2& sample);

    real SmithLambda(const vec3& w, real alpha_x, real alpha_y);

    real SmithGGXAnisotropic(const vec3& wo, const vec3& wi, real alpha_x, real alpha_y);

    real SmithGGXSeprate(const vec3& w, real alpha);

    std::optional<vec3> RefractDir(const vec3& w_i, const vec3& normal, const real eta);
}

FM_ENGINE_END

#endif
