#include "material_common.h"
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

namespace mat_func{
    real GTR2Anisotropic(const vec3& wh, real alpha_x, real alpha_y)
    {
        const real phi = GetSphericalPhi(wh);
        const real theta = GetSphericalTheta(wh);

        const real a = std::cos(phi) / alpha_x;
        const real b = std::sin(phi) / alpha_y;

        const real sin_theta = std::sin(theta);
        const real cos_theta = std::cos(theta);
        const real den = sin_theta * sin_theta * (a * a + b * b) + cos_theta * cos_theta;

        return 1.0_r / (PI * alpha_x * alpha_y * den * den);
    }

    vec3 SampleGTR2(real alpha_x, real alpha_y, const vec2& sample)
    {
        // Ref1: https://zhuanlan.zhihu.com/p/57771965?from_voters_page=true
        // Ref2: https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        real sin_phi = alpha_y * std::sin(2 * PI * sample.x);
        real cos_phi = alpha_x * std::cos(2 * PI * sample.x);
        const real norm = 1 / std::sqrt(sin_phi * sin_phi + cos_phi * cos_phi);
        sin_phi *= norm;
        cos_phi *= norm;

        const real a = cos_phi / alpha_x;
        const real b = sin_phi / alpha_y;
        const real psi_phi = a * a + b * b;

        const real cos_theta = std::sqrt(psi_phi * (1 - sample.y)
                / ((1 - psi_phi) * sample.y + psi_phi));

        const real sin_theta = std::sqrt(
                (std::max<real>)(0, 1 - cos_theta * cos_theta));

        const vec3 wh = vec3(sin_theta * cos_phi,sin_theta * sin_phi, cos_theta);
        return glm::normalize(wh);
    }

    real GTR1GGX(const vec3& wh, real alpha)
    {
        const real theta = GetSphericalTheta(wh);
        const real sin_theta = std::sin(theta);
        const real cos_theta = std::cos(theta);
        const real sin_theta_2 = sin_theta * sin_theta;
        const real cos_theta_2 = cos_theta * cos_theta;
        const real alpha2 = alpha * alpha;
        const real den = 2.0_r * PI * std::log(alpha) * (alpha2 * cos_theta_2 + sin_theta_2);
        return (alpha2 - 1.0_r) / den;
    }

    vec3 SampleGTR1(real alpha, const vec2& sample)
    {
        const real phi = 2.0_r * PI * sample.x;
        const real cos_theta = std::sqrt((std::pow(alpha, 2 - 2 * sample.y) - 1)
                / (alpha * alpha - 1));
        const real sin_theta = std::sqrt(
                (std::max<real>)(0, 1 - cos_theta * cos_theta));

        const vec3 wh = vec3(sin_theta * std::cos(phi),
                             sin_theta * std::sin(phi), cos_theta);

        return glm::normalize(wh);
    }

    real SmithLambda(const vec3& w, real alpha_x, real alpha_y)
    {
        const real phi = GetSphericalPhi(w);
        const real theta = GetSphericalTheta(w);

        const real a = std::cos(phi) * alpha_x;
        const real b = std::sin(phi) * alpha_y;

        const real tan_theta = std::tan(theta);

        return -0.5_r + 0.5_r * std::sqrt(1.0_r + (a * a + b * b) * tan_theta * tan_theta);
    }

    real SmithGGXAnisotropic(const vec3& wo, const vec3& wi, real alpha_x, real alpha_y)
    {
        const real lambda_i = SmithLambda(wi, alpha_y, alpha_y) + 1.0_r;
        const real lambda_o = SmithLambda(wo, alpha_y, alpha_y) + 1.0_r;
        return 1.0_r / lambda_i / lambda_o;
    }

    real SmithGGXSeprate(const vec3& w, real alpha)
    {
        const real theta = GetSphericalTheta(w);
        const real tan_theta = std::tan(theta);

        if (tan_theta < eps_pdf) {
            return 1.0_r;
        }

        const real den = tan_theta * alpha;
        return 2.0_r / (1.0_r + std::sqrt(1.0_r + den * den));
    }

    std::optional<vec3> RefractDir(const vec3& w_i, const vec3& normal, const real eta)
    {
        real cos_i = glm::dot(normal, w_i);
        real sin2i = std::max(0.0_r, 1.0_r - cos_i * cos_i);
        real sin2t = eta * eta * sin2i;

        // total internal reflection
        if (sin2t >= 1.0_r) {
            return std::nullopt;
        }

        real cos_t = std::sqrt(1.0_r - sin2t);
        return glm::normalize(eta * (-w_i) + (eta * cos_i - cos_t) * normal);
    }
}

FM_ENGINE_END
