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
        return vec3(0.0_r, 0.0_r, 0.0_r);
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
    real eta_i;
    real eta_o;
public:
    SpecularReflection(const vec3& color, real eta_in, real eta_out) : albedo(color), eta_i(eta_in), eta_o(eta_out)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        auto frenel = makeSP<DielectricFresnel>(eta_i, eta_o);
        hit_point.bsdf = makeSP<SpecularReflectionBSDF>(hit_point, albedo, frenel);
    };
};

FM_ENGINE_END