#include <engine/core/material.h>

FM_ENGINE_BEGIN

class LambertDiffuseBSDF : public BSDF
{
private:
    vec3 color;
public:
    LambertDiffuseBSDF(const HitPoint& hit_point, const vec3& albedo);
    ~LambertDiffuseBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
};

LambertDiffuseBSDF::LambertDiffuseBSDF(const HitPoint& hit_point, const vec3 &albedo) : BSDF(hit_point)
{
    color = albedo * InvPI;
}

vec3 LambertDiffuseBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    return color;
}

class LambertDiffuse : public Material
{
private:
    vec3 albedo;
public:
    LambertDiffuse(const vec3& color) : albedo(color)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        hit_point.bsdf = makeSP<LambertDiffuseBSDF>(hit_point, albedo);
    };
};

FM_ENGINE_END