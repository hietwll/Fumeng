#include <engine/core/material.h>
#include <engine/core/texture.h>

#include <utility>

FM_ENGINE_BEGIN

class BaseBXDF
{
public:
    BaseBXDF() = default;
    virtual ~BaseBXDF();
    virtual vec3 eval(const vec3 &wo, const vec3 &wi) const;
    virtual real pdf(const vec3 &wo, const vec3 &wi) const;
    virtual vec3 sample(const vec3& wo_w, const vec2& samples) const;
};

class DisneySpecularReflection : public BaseBXDF
{
private:
    vec3 m_basecolor;
    real m_specularTint;
    real m_metallic;
public:
    vec3 eval(const vec3 &wo, const vec3 &wi) const override
    {
        bool is_reflection = wo.z > 0 && wi.z > 0;
        if (!is_reflection) {
            return black;
        }

        // Fresnel

    }
};

class DisneyClearCoat : public BaseBXDF
{

};

class DisneyRoughSpecularTransmission : public BaseBXDF
{

};

class DisneySpecularTransmission : public BaseBXDF
{

};

class DisneyRetroReflection : public BaseBXDF
{

};

class DisneySheen : public BaseBXDF
{

};

class DisneyDiffuse : public BaseBXDF
{

};

class DisneyFakeSS : public BaseBXDF
{

};

class DisneyLambertianTransmission : public BaseBXDF
{

};

class DisneyBSDF : public BSDF
{
private:
    vec3 m_basecolor;
    real m_metallic;
    real m_specular;
    real m_specularTint;
    real m_roughness;
    real m_anisotropic;
    real m_sheen;
    real m_sheenTint;
    real m_clearcoat;
    real m_clearcoatGloss;
    real m_specTrans;
    real m_diffTrans;
    real m_flatness;
    bool m_thin;
public:
    DisneyBSDF(const HitPoint& hit_point,
       const vec3& basecolor,
       const real metallic,
       const real specular,
       const real specularTint,
       const real roughness,
       const real anisotropic,
       const real sheen,
       const real sheenTint,
       const real clearcoat,
       const real clearcoatGloss,
       const real specTrans,
       const real diffTrans,
       const real flatness,
       const bool thin);
    ~DisneyBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
    real PdfLocal(const vec3& wo, const vec3& wi) const override;
    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const override;
};

DisneyBSDF::DisneyBSDF(const HitPoint &hit_point, const vec3 &basecolor, const real metallic, const real specular,
                       const real specularTint, const real roughness, const real anisotropic, const real sheen,
                       const real sheenTint, const real clearcoat, const real clearcoatGloss, const real specTrans,
                       const real diffTrans, const real flatness, const bool thin)
                       : BSDF(hit_point)
{
    m_basecolor = basecolor;
    m_metallic = metallic;
    m_specular = specular;
    m_specularTint = specularTint;
    m_roughness = roughness;
    m_anisotropic = anisotropic;
    m_sheen = sheen;
    m_sheenTint = sheenTint;
    m_clearcoat = clearcoat;
    m_clearcoatGloss = clearcoatGloss;
    m_specTrans = specTrans;
    m_diffTrans = diffTrans;
    m_flatness = flatness;
    m_thin = thin;
}

vec3 DisneyBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{

}

class DisneyMaterial : public Material
{
private:
    SP<Texture> m_basecolor;
    SP<Texture> m_metallic;
    SP<Texture> m_specular;
    SP<Texture> m_specularTint;
    SP<Texture> m_roughness;
    SP<Texture> m_anisotropic;
    SP<Texture> m_sheen;
    SP<Texture> m_sheenTint;
    SP<Texture> m_clearcoat;
    SP<Texture> m_clearcoatGloss;
    SP<Texture> m_specTrans;
    SP<Texture> m_diffTrans;
    SP<Texture> m_flatness;
    bool m_thin;

public:
    DisneyMaterial(
        SP<Texture> basecolor,
        SP<Texture> metallic,
        SP<Texture> specular,
        SP<Texture> specularTint,
        SP<Texture> roughness,
        SP<Texture> anisotropic,
        SP<Texture> sheen,
        SP<Texture> sheenTint,
        SP<Texture> clearcoat,
        SP<Texture> clearcoatGloss,
        SP<Texture> specTrans,
        SP<Texture> diffTrans,
        SP<Texture> flatness,
        const bool thin)
    {
        m_basecolor = basecolor;
        m_metallic = metallic;
        m_specular = specular;
        m_specularTint = specularTint;
        m_roughness = roughness;
        m_anisotropic = anisotropic;
        m_sheen = sheen;
        m_sheenTint = sheenTint;
        m_clearcoat = clearcoat;
        m_clearcoatGloss = clearcoatGloss;
        m_specTrans = specTrans;
        m_diffTrans = diffTrans;
        m_flatness = flatness;
        m_thin = thin;
    }

    void CreateBSDF(HitPoint &hit_point) const override
    {
        const vec3 basecolor = m_basecolor->Sample(hit_point.uv);
        const real metallic = m_metallic->Sample(hit_point.uv).x;
        const real specular = m_specular->Sample(hit_point.uv).x;
        const real specularTint = m_specularTint->Sample(hit_point.uv).x;
        const real roughness = m_roughness->Sample(hit_point.uv).x;
        const real anisotropic = m_anisotropic->Sample(hit_point.uv).x;
        const real sheen = m_sheen->Sample(hit_point.uv).x;
        const real sheenTint = m_sheenTint->Sample(hit_point.uv).x;
        const real clearcoat = m_clearcoat->Sample(hit_point.uv).x;
        const real clearcoatGloss = m_clearcoatGloss->Sample(hit_point.uv).x;
        const real specTrans = m_specTrans->Sample(hit_point.uv).x;
        const real diffTrans = m_diffTrans->Sample(hit_point.uv).x;
        const real flatness = m_flatness->Sample(hit_point.uv).x;
        const bool thin = m_thin;
        hit_point.bsdf = MakeSP<DisneyBSDF>(hit_point,
                                            basecolor,
                                            metallic,
                                            specular,
                                            specularTint,
                                            roughness,
                                            anisotropic,
                                            sheen,
                                            sheenTint,
                                            clearcoat,
                                            clearcoatGloss,
                                            specTrans,
                                            diffTrans,
                                            flatness,
                                            thin);
    };
};

FM_ENGINE_END
