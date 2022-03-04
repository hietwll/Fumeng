#include <engine/core/material.h>
#include <engine/core/texture.h>
#include <engine/create/texture.h>

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
    SP<Texture> diffuse_color;
public:
    explicit LambertDiffuse(const LambertDiffuseConfig& config)
    {
        diffuse_color = CreateTexture(config.diffuse_color);
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        const vec3 sampled_color = diffuse_color->Sample(hit_point.uv);
        hit_point.bsdf = MakeSP<LambertDiffuseBSDF>(hit_point, sampled_color);
    };
};

SP<Material> CreateLambertDiffuse(const LambertDiffuseConfig& config)
{
    return MakeSP<LambertDiffuse>(config);
}

FM_ENGINE_END