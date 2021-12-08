#include <engine/core/material.h>
#include <engine/core/texture.h>
#include <engine/core/utils.h>
#include <engine/core/fresnel.h>

FM_ENGINE_BEGIN

inline vec3 SchlickFresnel(const vec3& F0, real cos_i)
{
    const real x = 1.0_r - cos_i;
    const real x2 = x * x;
    return F0 + (white - F0) * x2 * x2 * x;
}

inline real GTR2Anisotropic(const vec3& wh, real alpha_x, real alpha_y)
{
    const real phi = GetSphericalPhi(wh);
    const real theta = GetSphericalTheta(wh);

    const real a = std::cos(phi) / alpha_x;
    const real b = std::sin(phi) / alpha_y;

    const real tan_theta = std::tan(theta);
    const real den = tan_theta * tan_theta * (a * a + b * b) + 1.0_r;

    return 1.0_r / (PI * alpha_x * alpha_y * den * den);
}

inline real SmithLambda(const vec3& w, real alpha_x, real alpha_y)
{
    const real phi = GetSphericalPhi(w);
    const real theta = GetSphericalTheta(w);

    const real a = std::cos(phi) * alpha_x;
    const real b = std::sin(phi) * alpha_y;

    const real tan_theta = std::tan(theta);

    return -0.5_r + 0.5_r * std::sqrt(1.0_r + (a * a + b * b) * tan_theta * tan_theta);
}

inline real SmithGGXAnisotropic(const vec3& wo, const vec3& wi, real alpha_x, real alpha_y)
{
    const real lambda_i = SmithLambda(wi, alpha_y, alpha_y) + 1.0_r;
    const real lambda_o = SmithLambda(wo, alpha_y, alpha_y) + 1.0_r;
    return 1.0_r / lambda_i / lambda_o;
}

class BaseBXDF
{
public:
    BaseBXDF() = default;
    virtual ~BaseBXDF();
    virtual vec3 Eval(const vec3 &wo, const vec3 &wi) const;
    virtual real Pdf(const vec3 &wo, const vec3 &wi) const;
    virtual vec3 Sample(const vec3& wo_w, const vec2& samples) const;
};

/**
 * Piecewise Dielectric Fresnel Function in Burley 2015's Paper, Eq.(8)
 */
class DisneyDielectricFresnel
{
private:
    UP<DielectricFresnel> m_fresnel;
    real m_ior;

public:
    DisneyDielectricFresnel(real ior)
    {
        m_ior = ior;
        m_fresnel = MakeUP<DielectricFresnel>(1.0_r, ior);
    }

    vec3 CalFr(real cos_i) const
    {
        const real cos_t2 = 1.0_r - (1.0_r - cos_i * cos_i) / m_ior / m_ior;

        if (cos_t2 > 0) {
            return m_fresnel->CalFr(cos_i);
        } else {
            return white;
        }
    }
};

class DisneySpecularReflection : public BaseBXDF
{
private:
    vec3 m_basecolor;
    real m_specular;
    real m_specularTint;
    real m_metallic;
    real m_ior;
    real m_alpha_x;
    real m_alpha_y;

public:
    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        bool is_reflection = wo.z > 0 && wi.z > 0;
        if (!is_reflection) {
            return black;
        }

        const vec3 wh = glm::normalize(wo + wi); // half vector
        const real cos_o = CosDir(wo);
        const real cos_i = CosDir(wi);

        // theta_d is the “difference” angle between light (i.e. wi) and the half vector
        // note wh is assumed to be normal direction in fresnel calculation
        const real cos_d = glm::dot(wi, wh);

        // Fresnel
        vec3 Cspec = Lerp(white, ToTint(m_basecolor), m_specularTint);
        Cspec = Lerp(Cspec, m_basecolor, m_metallic);

        DisneyDielectricFresnel dielectric_fresnel(m_ior);
        const vec3 f_dielectric = Cspec * dielectric_fresnel.CalFr(cos_d);
        const vec3 f_conduct = SchlickFresnel(Cspec, cos_d);
        const vec3 f = Lerp(m_specular * f_dielectric, f_conduct, m_metallic);

        // Microfacet normal distribution
        const real d = GTR2Anisotropic(wh, m_alpha_x, m_alpha_y);

        // Geometry mask
        const real g = SmithGGXAnisotropic(wo, wi, m_alpha_x, m_alpha_y);

        return f * d * g / std::abs(4.0_r * cos_i * cos_o);
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
    real m_ior;
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
       const real ior,
       const bool thin);
    ~DisneyBSDF() = default;
    vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const override;
    real PdfLocal(const vec3& wo, const vec3& wi) const override;
    BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const override;
};

DisneyBSDF::DisneyBSDF(const HitPoint &hit_point, const vec3 &basecolor, const real metallic, const real specular,
                       const real specularTint, const real roughness, const real anisotropic, const real sheen,
                       const real sheenTint, const real clearcoat, const real clearcoatGloss, const real specTrans,
                       const real diffTrans, const real flatness, const real ior, const bool thin)
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
    m_ior = ior;
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
    SP<Texture> m_ior;
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
        SP<Texture> ior,
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
        m_ior = ior;
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
        const real ior = m_ior->Sample(hit_point.uv).x;
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
                                            ior,
                                            thin);
    };
};

FM_ENGINE_END
