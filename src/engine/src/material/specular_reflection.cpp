#include <engine/core/material.h>
#include <engine/core/fresnel.h>

FM_ENGINE_BEGIN

class SpecularReflectionBSDF : public BSDF
{
private:
    vec3 color;
    SP<Fresnel> fresnel;
public:
    SpecularReflectionBSDF(const HitPoint& hit_point, const vec3& albedo, SP<Fresnel> fresnel_)
    : color(albedo), fresnel(fresnel_), BSDF(hit_point)
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

    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const override
    {
        vec3 wo = WorldToShading(wo_w);
        vec3 wi = vec3(-wo.x, -wo.y, wo.z);

        real pdf = 1.0_r;
        real cos_i = CosDir(wi);
        vec3 f = fresnel->CalFr(cos_i) * color / std::abs(cos_i);
        return {f, ShadingToWorld(wi), pdf, true};
    };
};

class SpecularReflection : public Material
{
private:
    vec3 albedo;
    vec3 eta_i_; // incident media
    vec3 eta_t_; // transmitted (reflected) media
    vec3 k_; // absorption coefficient
public:
    SpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k)
    : albedo(color), eta_i_(eta_i), eta_t_(eta_t), k_(k)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        auto fresnel = MakeSP<ConductorFresnel>(eta_i_, eta_t_, k_);
        hit_point.bsdf = MakeSP<SpecularReflectionBSDF>(hit_point, albedo, fresnel);
    };
};

SP<Material> CreateSpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k)
{
    return MakeSP<SpecularReflection>(color, eta_i, eta_t, k);
}

FM_ENGINE_END