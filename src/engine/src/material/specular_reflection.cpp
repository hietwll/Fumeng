#include <engine/core/material.h>
#include <engine/core/fresnel.h>

FM_ENGINE_BEGIN

class SpecularReflectionBSDF : public BSDF
{
private:
    vec3 m_basecolor;
    SP<Fresnel> m_fresnel;
public:
    SpecularReflectionBSDF(const HitPoint& hit_point, const vec3& basecolor, SP<Fresnel> fresnel)
    : m_basecolor(basecolor), m_fresnel(fresnel), BSDF(hit_point)
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
        vec3 wi = vec3(-wo.x, -wo.y, wo.z);

        real pdf = 1.0_r;
        real cos_i = CosDir(wi);
        vec3 f = m_fresnel->CalFr(cos_i) * m_basecolor / std::abs(cos_i);
        return {f, ShadingToWorld(wi), pdf, true};
    };

    vec3 GetAlbedo() const override
    {
        return m_basecolor;
    };
};

class SpecularReflection : public Material
{
private:
    vec3 m_basecolor;
    vec3 m_eta_i; // incident media
    vec3 m_eta_t; // transmitted (reflected) media
    vec3 m_k; // absorption coefficient
public:
    SpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k)
    : m_basecolor(color), m_eta_i(eta_i), m_eta_t(eta_t), m_k(k)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        auto fresnel = MakeSP<ConductorFresnel>(m_eta_i, m_eta_t, m_k);
        hit_point.bsdf = MakeSP<SpecularReflectionBSDF>(hit_point, m_basecolor, fresnel);
    };
};

SP<Material> CreateSpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k)
{
    return MakeSP<SpecularReflection>(color, eta_i, eta_t, k);
}

FM_ENGINE_END