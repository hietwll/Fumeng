#include <engine/core/material.h>

FM_ENGINE_BEGIN

class LambertDiffuseBSDF : public BSDF
{
private:
    vec3 color;
public:
    LambertDiffuseBSDF(const HitPoint& hit_point, const vec3& albedo);
    ~LambertDiffuseBSDF() = default;
    vec3 Cal_f(const vec3& wo, const vec3& wi) const override;
    real Pdf(const vec3& wo, const vec3& wi) const override;
    BSDFSampleInfo SampleBSDF(const vec3& wo, const vec3& samples) const override;
};

LambertDiffuseBSDF::LambertDiffuseBSDF(const HitPoint& hit_point, const vec3 &albedo) : BSDF(hit_point)
{
    color = albedo * InvPI;
}

vec3 LambertDiffuseBSDF::Cal_f(const vec3 &wo, const vec3 &wi) const
{
    return color;
}

class LambertDiffuse : public Material
{

};

FM_ENGINE_END