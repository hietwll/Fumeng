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
    real m_diffTrans;
    real m_flatness;
    real m_ior;
    bool m_thin;

    // derived parameters
    real m_alpha_x;
    real m_alpha_y;
    vec3 m_ctint;
    real m_clearcoat_roughness;

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
               real diffTrans,
               real flatness,
               real ior,
               bool thin);
    ~DisneyBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
    real PdfLocal(const vec3& wo, const vec3& wi) const override;
    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const override;
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
    virtual real Pdf(const vec3 &wo, const vec3 &wi) const = 0;
    virtual vec3 Sample(const vec3& wo_w, const vec2& samples) const = 0;
};

FM_ENGINE_END

#endif


