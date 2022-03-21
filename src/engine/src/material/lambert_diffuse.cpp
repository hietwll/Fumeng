#include <engine/core/material.h>
#include <engine/core/texture.h>
#include <engine/create/texture.h>

#include <utility>

FM_ENGINE_BEGIN

class LambertDiffuseBSDF : public BSDF
{
private:
    vec3 m_basecolor;

public:
    LambertDiffuseBSDF(const HitPoint& hit_point, const vec3& basecolor);
    ~LambertDiffuseBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
    vec3 GetAlbedo() const override;
};

LambertDiffuseBSDF::LambertDiffuseBSDF(const HitPoint& hit_point, const vec3 &basecolor) :
BSDF(hit_point),
m_basecolor(basecolor)
{
}

vec3 LambertDiffuseBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    return m_basecolor * InvPI;
}

vec3 LambertDiffuseBSDF::GetAlbedo() const
{
    return m_basecolor;
}

class LambertDiffuse : public Material
{
private:
    SP<Texture> m_basecolor;
public:
    explicit LambertDiffuse(const LambertDiffuseConfig& config)
    {
        m_basecolor = CreateTexture(config.basecolor);
    };

    void CreateBSDF(HitPoint &hit_point) const override
    {
        const vec3 sampled_color = m_basecolor->Sample(hit_point.uv);
        hit_point.bsdf = MakeSP<LambertDiffuseBSDF>(hit_point, sampled_color);
    };
};

SP<Material> CreateLambertDiffuse(const LambertDiffuseConfig& config)
{
    return MakeSP<LambertDiffuse>(config);
}

FM_ENGINE_END