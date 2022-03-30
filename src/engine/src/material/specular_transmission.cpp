#include <engine/core/material.h>
#include <engine/core/fresnel.h>
#include "material_common.h"

FM_ENGINE_BEGIN

class SpecularTransmissionBSDF : public BSDF
{
private:
    vec3 m_reflect_color;
    vec3 m_refract_color;
    SP<DielectricFresnel> m_fresnel;
public:
    SpecularTransmissionBSDF(const HitPoint& hit_point, const vec3& reflect_color, const vec3& refract_color, SP<DielectricFresnel> fresnel)
    : m_reflect_color(reflect_color), m_refract_color(refract_color), m_fresnel(fresnel), BSDF(hit_point)
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

    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec3 &samples) const override
    {
        vec3 wo = WorldToShading(wo_w);
        real fr = (m_fresnel->CalFr(CosDir(wo))).x;

        // calculate specular reflection
        if (samples.x < fr) {
            vec3 wi = vec3(-wo.x, -wo.y, wo.z);

            real pdf = fr;
            real cos_i = CosDir(wi);
            vec3 f = m_fresnel->CalFr(cos_i) * m_reflect_color / std::abs(cos_i);
            return {f, ShadingToWorld(wi), pdf, true};
        }

        // calculate specular refraction
        real eta_i = m_fresnel->EtaI();
        real eta_t = m_fresnel->EtaT();
        vec3 normal = vec3(0.0_r, 0.0_r, 1.0_r);
        if (CosDir(wo) < 0) {
            std::swap(eta_i, eta_t);
            normal = -normal;
        }

        real eta = eta_i / eta_t;

        // get refracted direction
        auto wt = mat_func::RefractDir(wo, normal, eta);
        if (!wt) {
            return {black, black, 0.0_r, true};
        }

        real pdf = 1.0_r - fr;
        real cos_t = CosDir(wt.value());
        vec3 f = eta * eta * m_refract_color * (1.0_r - fr) / std::abs(cos_t);
        return {f, ShadingToWorld(wt.value()), pdf, true};
    };

    vec3 GetAlbedo() const override
    {
        return m_reflect_color;
    };
};

class SpecularTransmission : public Material
{
private:
    vec3 m_reflect_color;
    vec3 m_refract_color;
    real m_eta_i; // incident media
    real m_eta_t; // transmitted (reflected) media
public:
    SpecularTransmission(const SpecularTransmissionConfig& config) :
    m_reflect_color(config.reflect_color),
    m_refract_color(config.refract_color),
    m_eta_i(config.eta_i),
    m_eta_t(config.eta_t)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        auto fresnel = MakeSP<DielectricFresnel>(m_eta_i, m_eta_t);
        hit_point.bsdf = MakeSP<SpecularTransmissionBSDF>(hit_point, m_reflect_color, m_refract_color, fresnel);
    };
};

SP<Material> CreateSpecularTransmission(const SpecularTransmissionConfig& config)
{
    return MakeSP<SpecularTransmission>(config);
}

FM_ENGINE_END