#include <engine/core/texture.h>
#include <engine/core/utils.h>
#include <engine/core/fresnel.h>
#include "disney.h"

FM_ENGINE_BEGIN

inline real PowerFive(real x)
{
    const real x2 = x * x;
    return x2 * x2 * x;
}

inline vec3 SchlickFresnel(const vec3& F0, real cos_i)
{
    return F0 + (white - F0) * PowerFive(1.0_r - cos_i);
}

inline real SchlickFresnelScalar(const real& F0, real cos_i)
{
    return F0 + (1.0_r - F0) * PowerFive(1.0_r - cos_i);
}

inline real GTR2Anisotropic(const vec3& wh, real alpha_x, real alpha_y)
{
    const real phi = GetSphericalPhi(wh);
    const real theta = GetSphericalTheta(wh);

    const real a = std::cos(phi) / alpha_x;
    const real b = std::sin(phi) / alpha_y;

    const real sin_theta = std::sin(theta);
    const real cos_theta = std::cos(theta);
    const real den = sin_theta * sin_theta * (a * a + b * b) + cos_theta * cos_theta;

    return 1.0_r / (PI * alpha_x * alpha_y * den * den);
}

inline vec3 SampleGTR2(real alpha_x, real alpha_y, const vec2& sample)
{
    // Ref1: https://zhuanlan.zhihu.com/p/57771965?from_voters_page=true
    // Ref2: https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
    real sin_phi = alpha_y * std::sin(2 * PI * sample.x);
    real cos_phi = alpha_x * std::cos(2 * PI * sample.x);
    const real norm = 1 / std::sqrt(sin_phi * sin_phi + cos_phi * cos_phi);
    sin_phi *= norm;
    cos_phi *= norm;

    const real a = cos_phi / alpha_x;
    const real b = sin_phi / alpha_y;
    const real psi_phi = a * a + b * b;

    const real cos_theta = std::sqrt(psi_phi * (1 - sample.y)
            / ((1 - psi_phi) * sample.y + psi_phi));

    const real sin_theta = std::sqrt(
            (std::max<real>)(0, 1 - cos_theta * cos_theta));

    const vec3 wh = vec3(sin_theta * cos_phi,sin_theta * sin_phi, cos_theta);
    return glm::normalize(wh);
}

inline real GTR1GGX(const vec3& wh, real alpha)
{
    const real theta = GetSphericalTheta(wh);
    const real sin_theta = std::sin(theta);
    const real cos_theta = std::cos(theta);
    const real sin_theta_2 = sin_theta * sin_theta;
    const real cos_theta_2 = cos_theta * cos_theta;
    const real alpha2 = alpha * alpha;
    const real den = 2.0_r * PI * std::log(alpha) * (alpha2 * cos_theta_2 + sin_theta_2);
    return (alpha2 - 1.0_r) / den;
}

inline vec3 SampleGTR1(real alpha, const vec2& sample)
{
    const real phi = 2.0_r * PI * sample.x;
    const real cos_theta = std::sqrt((std::pow(alpha, 2 - 2 * sample.y) - 1)
            / (alpha * alpha - 1));
    const real sin_theta = std::sqrt(
            (std::max<real>)(0, 1 - cos_theta * cos_theta));

    const vec3 wh = vec3(sin_theta * std::cos(phi),
                         sin_theta * std::sin(phi), cos_theta);

    return glm::normalize(wh);
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

inline real SmithGGXSeprate(const vec3& w, real alpha)
{
    const real theta = GetSphericalTheta(w);
    const real tan_theta = std::tan(theta);

    if (tan_theta < eps_pdf) {
        return 1.0_r;
    }

    const real den = tan_theta * alpha;
    return 2.0_r / (1.0_r + std::sqrt(1.0_r + den * den));
}

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
public:
    DisneySpecularReflection(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return black;
        }

        const vec3 wh = glm::normalize(wo + wi); // half vector
        const real cos_o = CosDir(wo);
        const real cos_i = CosDir(wi);

        // theta_d is the “difference” angle between light (i.e. wi) and the half vector
        // note wh is assumed to be normal direction in fresnel calculation
        const real cos_d = glm::dot(wi, wh);

        // fresnel
        vec3 c_spec = Lerp(white, m_p->m_ctint, m_p->m_specularTint);
        c_spec = Lerp(c_spec, m_p->m_basecolor, m_p->m_metallic);

        DisneyDielectricFresnel dielectric_fresnel(m_p->m_ior);
        const vec3 f_dielectric = c_spec * dielectric_fresnel.CalFr(cos_d);
        const vec3 f_conduct = SchlickFresnel(c_spec, cos_d);
        const vec3 f = Lerp(m_p->m_specular * f_dielectric, f_conduct, m_p->m_metallic);

        // microfacet normal distribution
        const real d = GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);

        // geometry mask
        const real g = SmithGGXAnisotropic(wo, wi, m_p->m_alpha_x, m_p->m_alpha_y);

        return f * d * g / std::abs(4.0_r * cos_i * cos_o);
    }

    vec3 Sample(const vec3& wo_w, const vec2& samples) const override
    {
        const vec3 wh = SampleGTR2(m_p->m_alpha_x, m_p->m_alpha_y, samples);
        const vec3 wi = glm::normalize(2.0_r * glm::dot(wo_w, wh) * wh - wo_w);
        if (wi.z < 0) {
            return black;
        }
        return wi;
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return 0.0_r;
        }

        const vec3 wh = glm::normalize(wo + wi); // half vector
        const real cos_d = glm::dot(wi, wh);
        // microfacet normal distribution
        const real d = GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);
        return AbsCosDir(wh) * d / (4.0_r * cos_d);
    }
};

/**
 * DisneyDiffuse in Burley 2015's Paper, Eq.(4)
 */
class DisneyDiffuse : public BaseBXDF
{
public:
    DisneyDiffuse(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        const vec3 wh = glm::normalize(wo + wi);
        const real cos_o = CosDir(wo); // view dir
        const real cos_i = CosDir(wi); // light dir
        const real cos_d = glm::dot(wi, wh);

        // lambert
        const vec3 f_lambert = m_p->m_basecolor / PI;

        // retro reflection
        const real fl = PowerFive(1.0_r - cos_i);
        const real fv = PowerFive(1.0_r - cos_o);
        const real rr = 2.0_r * m_p->m_roughness * cos_d * cos_d;
        const vec3 f_retro = f_lambert * rr * (fl + fv + fl * fv * (rr - 1.0_r));

        // sheen
        const vec3 c_sheen = Lerp(white, m_p->m_ctint, m_p->m_sheenTint);
        const vec3 f_sheen = 4.0_r * m_p->m_sheen * c_sheen * PowerFive(1.0_r - cos_d);

        return f_lambert * (1.0_r - 0.5_r * fl) * (1.0_r - 0.5_r * fv) + f_retro + f_sheen;
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        return CosineWeightedHemiSphere(samples);
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return 0.0_r;
        }
        return std::abs(wi.z) * InvPI;
    }
};

class DisneyClearCoat : public BaseBXDF
{
public:
    DisneyClearCoat(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return black;
        }

        const vec3 wh = glm::normalize(wo + wi);
        const real cos_o = CosDir(wo); // view dir
        const real cos_i = CosDir(wi); // light dir
        const real cos_d = glm::dot(wi, wh);

        // fresnel
        const real f = SchlickFresnelScalar(0.04_r, cos_d);

        // microfacet normal distribution
        const real d = GTR1GGX(wh, m_p->m_clearcoat_roughness);

        // geometry mask
        const real g = SmithGGXSeprate(wi, 0.25_r) * SmithGGXSeprate(wo, 0.25_r);

        const real ret = m_p->m_clearcoat * f * d * g / std::abs(4.0_r * cos_i * cos_o);
        return vec3(ret);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        const vec3 wh = SampleGTR1(m_p->m_clearcoat_roughness, samples);
        const vec3 wi = glm::normalize(2.0_r * glm::dot(wo, wh) * wh - wo);
        if (wi.z < 0) {
            return black;
        }
        return wi;
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return 0.0_r;
        }

        const vec3 wh = glm::normalize(wo + wi); // half vector
        const real cos_d = glm::dot(wi, wh);
        // microfacet normal distribution
        const real d = GTR1GGX(wh, m_p->m_clearcoat_roughness);
        return AbsCosDir(wh) * d / (4.0_r * cos_d);
    }
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

class DisneyFakeSS : public BaseBXDF
{

};

class DisneyLambertianTransmission : public BaseBXDF
{

};


DisneyBSDF::DisneyBSDF(const HitPoint &hit_point, const vec3 &basecolor, const real metallic, const real specular,
                       const real specularTint, const real roughness, const real anisotropic, const real sheen,
                       const real sheenTint, const real clearcoat, const real clearcoatGloss, const real specTrans,
                       const real diffTrans, const real flatness, const real ior, const bool thin)
                       : BSDF(hit_point)
{
    // original parameters
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

    // derived parameters
    m_ctint = ToTint(m_basecolor);
    m_clearcoat_roughness = Lerp(0.1_r, 0.001_r, m_clearcoatGloss);

    const real aspect = m_anisotropic > 0 ?
            std::sqrt(1 - real(0.9) * m_anisotropic) : real(1);
    m_alpha_x = std::max(real(0.001), m_roughness * m_roughness / aspect);
    m_alpha_y = std::max(real(0.001), m_roughness * m_roughness * aspect);


    // weight
    w_diffuse = Clamp(RGBToLuminance(m_basecolor) * (1 - m_metallic), 0.3, 0.7);
    w_specular = (1.0 - w_diffuse) * 2 / (2 + m_clearcoat);
    w_clearcoat = (1.0 - w_diffuse) * m_clearcoat / (2 + m_clearcoat);

    const real w_sum_inv = 1.0_r / (w_diffuse + w_specular + w_clearcoat);

    w_diffuse = w_diffuse * w_sum_inv;
    w_specular = w_specular * w_sum_inv + w_diffuse;
    w_clearcoat = w_clearcoat * w_sum_inv + w_specular;

    // submodels
    m_disney_specular_reflection = MakeUP<DisneySpecularReflection>(this);
    m_disney_diffuse = MakeUP<DisneyDiffuse>(this);
    m_disney_clearcoat = MakeUP<DisneyClearCoat>(this);
}

vec3 DisneyBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    vec3 ret = black;
    if(wi.z <= 0 || wo.z <= 0) {
        return ret;
    }

    // diffuse
    if (m_metallic < 1.0_r) {
        ret += (1.0 - m_metallic) * m_disney_diffuse->Eval(wo, wi);
    }

    // specular
    ret += m_disney_specular_reflection->Eval(wo, wi);

    // clearcoat
    if (m_clearcoat > 0.0_r) {
        ret += m_disney_clearcoat->Eval(wo, wi);
    }

    return ret;
}

BSDFSampleInfo DisneyBSDF::SampleBSDF(const vec3 &wo_w, const vec2 &samples) const
{
    vec3 wo = WorldToShading(wo_w);

    real roulette = std::rand();
    vec3 wi = black;

    if (roulette < w_diffuse) {
        wi = m_disney_diffuse->Sample(wo, samples);
    } else if (roulette < w_specular) {
        wi = m_disney_specular_reflection->Sample(wo, samples);
    } else {
        wi = m_disney_clearcoat->Sample(wo, samples);
    }

    real pdf = PdfLocal(wo, wi);
    vec3 f = CalFuncLocal(wo, wi);
    return {f, ShadingToWorld(wi), pdf, false};
}

real DisneyBSDF::PdfLocal(const vec3 &wo, const vec3 &wi) const
{
    const real pdf_diffuse = w_diffuse * m_disney_diffuse->Pdf(wo, wi);
    const real pdf_specular = w_specular * m_disney_specular_reflection->Pdf(wo, wi);
    const real pdf_clearcoat = w_clearcoat * m_disney_clearcoat->Pdf(wo, wi);

    return pdf_diffuse + pdf_specular + pdf_clearcoat;
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

SP<Material> CreateDisneyMaterial(
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
    return MakeSP<DisneyMaterial>(
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
}

FM_ENGINE_END