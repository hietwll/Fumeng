#ifndef FUMENG_DISNEY_H
#define FUMENG_DISNEY_H

#include <engine/core/material.h>

FM_ENGINE_BEGIN

class DisneyBSDF : public BSDF
{
private:
    // friend BXDF
    friend class BaseBXDF;
    friend class DisneySpecularReflection;
    friend class DisneyDiffuse;
    friend class DisneyClearCoat;
    friend class DisneyMicrofacetTransmission;
    friend class DisneyLambertianTransmission;
    friend class DisneyFakeSS;

    // original parameters
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
    real m_specTransRoughness;
    real m_diffTrans;
    real m_flatness;
    real m_ior;
    bool m_thin;

    // derived parameters
    real m_alpha_x;
    real m_alpha_y;
    vec3 m_ctint;
    real m_clearcoat_roughness;
    real m_ior_r;
    real m_trans_alpha_x;
    real m_trans_alpha_y;
    vec3 m_cspec0;
    real m_diffuse_weight; // weight for evaluation

    // weight for sampling
    real m_w_diffuse_refl;
    real m_w_specular_refl;
    real m_w_clearcoat;
    real m_w_specular_trans;
    real m_w_diffuse_trans;

    //  cdf of weight for sampling
    real m_c_diffuse_refl;
    real m_c_specular_refl;
    real m_c_clearcoat;
    real m_c_specular_trans;

    // submodels
    UP<DisneySpecularReflection> m_disney_specular_reflection;
    UP<DisneyDiffuse> m_disney_diffuse;
    UP<DisneyClearCoat> m_disney_clearcoat;
    UP<DisneyMicrofacetTransmission> m_disney_specular_transmission;
    UP<DisneyMicrofacetTransmission> m_disney_rough_transmission;
    UP<DisneyLambertianTransmission> m_disney_lambert_transmission;
    UP<DisneyFakeSS> m_disney_fake_ss;


public:
    DisneyBSDF(const HitPoint& hit_point,
               const vec3& basecolor,
               real metallic,
               real specular,
               real specularTint,
               real roughness,
               real anisotropic,
               real sheen,
               real sheenTint,
               real clearcoat,
               real clearcoatGloss,
               real specTrans,
               real specTransRoughness,
               real diffTrans,
               real flatness,
               real ior,
               bool thin);
    ~DisneyBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
    real PdfLocal(const vec3& wo, const vec3& wi) const override;
    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec3 &samples) const override;
    BSDFSampleInfo SampleInfoFromWoWi(const vec3& wo, const vec3& wi) const;
    vec3 RouletteSample(const vec3& wo, real roulette, const vec3& samples) const;
};

class BaseBXDF
{
protected:
    const DisneyBSDF* m_p;

public:
    explicit BaseBXDF(const DisneyBSDF* disneyBSDF) {
        m_p = disneyBSDF;
    };
    virtual ~BaseBXDF() = default;
    virtual vec3 Eval(const vec3 &wo, const vec3 &wi) const = 0;
    virtual real Pdf(const vec3 &wo, const vec3 &wi) const
    {
        return 0.0_r;
    };
    virtual vec3 Sample(const vec3& wo, const vec2& samples) const
    {
        return black;
    };
    bool IsReflection(const vec3 &wo, const vec3 &wi) const
    {
        return wo.z * wi.z > 0;
    };
    bool IsRefraction(const vec3 &wo, const vec3 &wi) const
    {
        return wo.z * wi.z < 0;
    };
    bool IsInternalReflection(const vec3 &wo, const vec3 &wi) const
    {
        return wo.z < 0 && wi.z < 0;
    };
};

FM_ENGINE_END

#endif


