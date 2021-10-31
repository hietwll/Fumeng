#include <engine/core/material.h>
#include <engine/core/fresnel.h>

#include <optional>

FM_ENGINE_BEGIN

class SpecularTransmissionBSDF : public BSDF
{
private:
    vec3 reflect_color;
    vec3 refract_color;
    SP<DielectricFresnel> fresnel;
public:
    SpecularTransmissionBSDF(const HitPoint& hit_point, const vec3& reflect_color_, const vec3& refract_color_, SP<DielectricFresnel> fresnel_)
    : reflect_color(reflect_color_), refract_color(refract_color_), fresnel(fresnel_), BSDF(hit_point)
    {
    }

    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override
    {
        return {0.0_r, 0.0_r, 0.0_r};
    };

    real PdfLocal(const vec3& wo, const vec3& wi) const override
    {
        return 0.0_r;
    }

    std::optional<vec3> RefractDir(const vec3& w_i, const vec3& normal, const real eta) const
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

    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const override
    {
        vec3 wo = WorldToShading(wo_w);
        real fr = (fresnel->CalFr(CosDir(wo))).x;

        // calculate specular reflection
        if (samples.x < fr) {
            vec3 wi = vec3(-wo.x, -wo.y, wo.z);

            real pdf = fr;
            real cos_i = CosDir(wi);
            vec3 f = fresnel->CalFr(cos_i) * refract_color / std::abs(cos_i);
            return {f, ShadingToWorld(wi), pdf, true};
        }

        // calculate specular refraction
        real eta_i = fresnel->EtaI();
        real eta_t = fresnel->EtaT();
        vec3 normal = vec3(0.0_r, 0.0_r, 1.0_r);
        if (CosDir(wo) < 0) {
            std::swap(eta_i, eta_t);
            normal = -normal;
        }

        real eta = eta_i / eta_t;

        // get refracted direction
        auto wt = RefractDir(wo, normal, eta);
        if (!wt) {
            return {black, black, 0.0_r, true};
        }

        real pdf = 1.0_r - fr;
        real cos_t = CosDir(wt.value());
        vec3 f = eta * eta * refract_color * (1.0_r - fr) / std::abs(cos_t);
        return {f, ShadingToWorld(wt.value()), pdf, true};
    };
};

class SpecularTransmission : public Material
{
private:
    vec3 reflect_color;
    vec3 refract_color;
    real eta_i_; // incident media
    real eta_t_; // transmitted (reflected) media
public:
    SpecularTransmission(const vec3& reflect_color_, const vec3& refract_color_, const real eta_i, const real eta_t)
    : reflect_color(reflect_color_), refract_color(refract_color), eta_i_(eta_i), eta_t_(eta_t)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        auto fresnel = MakeSP<DielectricFresnel>(eta_i_, eta_t_);
        hit_point.bsdf = MakeSP<SpecularTransmissionBSDF>(hit_point, reflect_color, refract_color, fresnel);
    };
};

SP<Material> CreateSpecularTransmission(const vec3& reflect_color_, const vec3& refract_color_, const real eta_i, const real eta_t)
{
    return MakeSP<SpecularTransmission>(reflect_color_, refract_color_, eta_i, eta_t);
}

FM_ENGINE_END