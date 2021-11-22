#include <engine/core/material.h>
#include <engine/core/texture.h>

#include <utility>

FM_ENGINE_BEGIN

class LambertDiffuseBSDF : public BSDF
{
private:
    vec3 diffuse_color;
public:
    LambertDiffuseBSDF(const HitPoint& hit_point, const vec3& albedo);
    ~LambertDiffuseBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
};

LambertDiffuseBSDF::LambertDiffuseBSDF(const HitPoint& hit_point, const vec3 &albedo) : BSDF(hit_point)
{
    diffuse_color = albedo * InvPI;
}

vec3 LambertDiffuseBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    return diffuse_color;
}

class LambertDiffuse : public Material
{
private:
    SP<Texture> color;
public:
    explicit LambertDiffuse(SP<Texture> color_) : color(color_)
    {
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        const vec3 sampled_color = color->Sample(hit_point.uv);
        hit_point.bsdf = MakeSP<LambertDiffuseBSDF>(hit_point, sampled_color);
    };
};

SP<Material> CreateLambertDiffuse(const SP<Texture>& color)
{
    return MakeSP<LambertDiffuse>(color);
}

FM_ENGINE_END