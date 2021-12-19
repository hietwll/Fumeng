#include <engine/core/texture.h>
#include <engine/core/utils.h>
#include <engine/core/fresnel.h>
#include "disney.h"
#include "material_common.h"

FM_ENGINE_BEGIN

/**
 * Piecewise Dielectric Fresnel Function in Burley 2015's Paper, Eq.(8)
 */
class DisneyDielectricFresnel
{
private:
    UP<DielectricFresnel> m_fresnel;

public:
    DisneyDielectricFresnel(real ior)
    {
        m_fresnel = MakeUP<DielectricFresnel>(1.0_r, ior);
    }

    vec3 CalFr(real cos_i) const
    {
        return m_fresnel->CalFr(cos_i);
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

        DisneyDielectricFresnel dielectric_fresnel(m_p->m_ior);
        const vec3 f_dielectric = m_p->m_cspec0 * dielectric_fresnel.CalFr(cos_d);
        const vec3 f_conduct = mat_func::SchlickFresnel(m_p->m_cspec0, cos_d);
        const vec3 f = Lerp(m_p->m_specular * f_dielectric, f_conduct, m_p->m_metallic);

        // microfacet normal distribution
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);

        // geometry mask
        const real g = mat_func::SmithGGXAnisotropic(wo, wi, m_p->m_alpha_x, m_p->m_alpha_y);

        return f * d * g / std::abs(4.0_r * cos_i * cos_o);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        const vec3 wh = mat_func::SampleGTR2(m_p->m_alpha_x, m_p->m_alpha_y, samples);
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
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);
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
        const real fl = mat_func::PowerFive(1.0_r - cos_i);
        const real fv = mat_func::PowerFive(1.0_r - cos_o);
        const real rr = 2.0_r * m_p->m_roughness * cos_d * cos_d;
        const vec3 f_retro = f_lambert * rr * (fl + fv + fl * fv * (rr - 1.0_r));

        // sheen
        const vec3 c_sheen = Lerp(white, m_p->m_ctint, m_p->m_sheenTint);
        const vec3 f_sheen = 4.0_r * m_p->m_sheen * c_sheen
                * mat_func::PowerFive(1.0_r - cos_d);

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
        const real f = mat_func::SchlickFresnelScalar(0.04_r, cos_d);

        // microfacet normal distribution
        const real d = mat_func::GTR1GGX(wh, m_p->m_clearcoat_roughness);

        // geometry mask
        const real g = mat_func::SmithGGXSeprate(wi, 0.25_r)
                * mat_func::SmithGGXSeprate(wo, 0.25_r);

        const real ret = m_p->m_clearcoat * f * d * g / std::abs(4.0_r * cos_i * cos_o);
        return vec3(ret);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        const vec3 wh = mat_func::SampleGTR1(m_p->m_clearcoat_roughness, samples);
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
        const real d = mat_func::GTR1GGX(wh, m_p->m_clearcoat_roughness);
        return AbsCosDir(wh) * d / (4.0_r * cos_d);
    }
};

class DisneyRoughSpecularTransmission : public BaseBXDF
{

};

class DisneySpecularTransmission : public BaseBXDF
{
public:
    DisneySpecularTransmission(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsRefraction(wo, wi)) {
            return black;
        }

        // refraction is not reciprocal, in/out direction influences relative IOR
        const real eta = wo.z > 0 ? m_p->m_ior : m_p->m_ior_r;

        // half direction of refraction, Burley 2015, Eq(1)
        vec3 wh = glm::normalize(wo + eta * wi);

        // wh is assumed to be in up hemisphere
        if (wh.z < 0) {
            wh = -wh;
        }

        // different from Brent Burley's 2015 paper, here wo is light direction
        // wi is view direction
        const real cos_o = CosDir(wo);
        const real cos_i = CosDir(wi);

        // theta_d is the “difference” angle between light (i.e. wi) and the half vector
        // note wh is assumed to be normal direction in fresnel calculation
        const real cos_d_o = glm::dot(wo, wh);
        const real cos_d_i = glm::dot(wi, wh);

        DisneyDielectricFresnel dielectric_fresnel(m_p->m_ior);
        const real f = dielectric_fresnel.CalFr(cos_d_o).x;

        // microfacet normal distribution
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_trans_alpha_x,
                                                 m_p->m_trans_alpha_y);

        // geometry mask
        const real g = mat_func::SmithGGXAnisotropic(wo, wi, m_p->m_trans_alpha_x,
                                                     m_p->m_trans_alpha_y);

        const real den = eta * cos_d_i + cos_d_o;

        // Burley 2015, Eq(2)
        const real f_t = (1.0_r - f) * d * g * cos_d_o * cos_d_i / (cos_i * cos_o * den * den);

        /**
         * 1. when ray goes from outside to inside (wo.z > 0), i.e. when look from out side to inside
         * we track diffuse, specular, clearcoat, sheen, transmission etc. so the coef for transmission
         * is m_specTrans.
         * 2. when ray goes from inside to outside, we only track specular and transmission, there is no
         * diffuse, so transmission has total share.
         */
        const real trans_coeff = wo.z > 0 ? m_p->m_specTrans : 1.0_r;

        return trans_coeff * (1.0_r - m_p->m_metallic) * m_p->m_basecolor * std::abs(f_t);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        const vec3 wh = mat_func::SampleGTR2(m_p->m_trans_alpha_x, m_p->m_trans_alpha_y, samples);
        // wh is assumed to be in up hemisphere
        if (wh.z <= 0) {
            spdlog::error("Sampled transmission normal direction should be in up hemisphere.");
            return black;
        }

        // eta for RefractDir should be incident IOR / transmitted IOR
        const real eta = wo.z > 0 ? m_p->m_ior_r : m_p->m_ior;

        // norm for RefractDir should be in same hemisphere
        const vec3 norm = glm::dot(wo, wh) > 0 ? wh : -wh;

        // get transmitted direction
        const auto wt = mat_func::RefractDir(wo, norm, eta);

        // total internal reflection
        if(!wt) {
            return black;
        }

        return wt.value();
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsRefraction(wo, wi)) {
            return 0.0_r;
        }

        // refraction is not reciprocal
        const real eta = wo.z > 0 ? m_p->m_ior : m_p->m_ior_r;

        // half direction of refraction, Burley 2015, Eq(1)
        vec3 wh = glm::normalize(wo + eta * wi);

        // wh is assumed to be in up hemisphere
        if (wh.z < 0) {
            wh = -wh;
        }

        // theta_d is the “difference” angle between light (i.e. wi) and the half vector
        // note wh is assumed to be normal direction in fresnel calculation
        const real cos_d_o = glm::dot(wo, wh);
        const real cos_d_i = glm::dot(wi, wh);

        // microfacet normal distribution
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_trans_alpha_x,
                                                 m_p->m_trans_alpha_y);

        const real den = eta * cos_d_i + cos_d_o;
        const real dwh_dwi = eta * eta * cos_d_i / den / den;
        return std::abs(cos_d_i * d * dwh_dwi);
    }
};

class DisneyFakeSS : public BaseBXDF
{

};

class DisneyLambertianTransmission : public BaseBXDF
{

};


DisneyBSDF::DisneyBSDF(const HitPoint &hit_point,
                       const vec3 &basecolor,
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
                       const real specTransRoughness,
                       const real diffTrans,
                       const real flatness,
                       const real ior,
                       const bool thin)
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
    m_specTransRoughness = specTransRoughness;
    m_diffTrans = diffTrans;
    m_flatness = flatness;
    m_ior = ior;
    m_thin = thin;

    // derived parameters
    m_ctint = ToTint(m_basecolor);
    m_clearcoat_roughness = Lerp(0.1_r, 0.0_r, m_clearcoatGloss);
    m_clearcoat_roughness *= m_clearcoat_roughness;
    m_clearcoat_roughness = m_clearcoat_roughness > 0.0001_r ? m_clearcoat_roughness : 0.0001_r;
    m_ior_r = 1.0_r / m_ior;
    m_cspec0 = Lerp(white, m_ctint, m_specularTint);
    m_cspec0 = Lerp(m_cspec0, m_basecolor, m_metallic);

    const real aspect = m_anisotropic > 0 ?
            std::sqrt(1 - real(0.9) * m_anisotropic) : real(1);
    m_alpha_x = std::max(real(0.001), m_roughness * m_roughness / aspect);
    m_alpha_y = std::max(real(0.001), m_roughness * m_roughness * aspect);

    m_trans_alpha_x = std::max(real(0.001), m_specTransRoughness * m_specTransRoughness / aspect);
    m_trans_alpha_y = std::max(real(0.001), m_specTransRoughness * m_specTransRoughness * aspect);


    // weight
    const real basecolor_lum = RGBToLuminance(m_basecolor);
    const real cspec0_lum = RGBToLuminance(m_cspec0);
    m_w_diffuse_refl = basecolor_lum * (1.0_r - m_metallic) * (1.0_r - m_specTrans);
    m_w_specular_refl = cspec0_lum * 2.0_r;
    m_w_clearcoat = m_clearcoat;
    m_w_specular_trans = basecolor_lum * (1.0_r - m_metallic) * specTrans;

    const real w_sum_inv = 1.0_r / (m_w_diffuse_refl + m_w_specular_refl
            + m_w_clearcoat + m_w_specular_trans);

    m_w_diffuse_refl = m_w_diffuse_refl * w_sum_inv;
    m_w_specular_refl = m_w_specular_refl * w_sum_inv + m_w_diffuse_refl;
    m_w_clearcoat = m_w_clearcoat * w_sum_inv + m_w_specular_refl;
    m_w_specular_trans = m_w_specular_trans * w_sum_inv + m_w_clearcoat;

    // submodels
    m_disney_specular_reflection = MakeUP<DisneySpecularReflection>(this);
    m_disney_diffuse = MakeUP<DisneyDiffuse>(this);
    m_disney_clearcoat = MakeUP<DisneyClearCoat>(this);
    m_disney_specular_transmission = MakeUP<DisneySpecularTransmission>(this);
}

vec3 DisneyBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    vec3 ret = black;
    vec3 diffuse = black;
    vec3 specular = black;
    vec3 transmission = black;
    vec3 clearcoat = black;

    if (wo.z * wi.z < 0 && m_specTrans > 0.0_r) {
        transmission = m_disney_specular_transmission->Eval(wo, wi);
        return transmission;
    }

    // diffuse
    if (m_metallic < 1.0_r && m_specTrans < 1.0_r) {
        diffuse = (1.0_r - m_metallic) * (1.0_r - m_specTrans) * m_disney_diffuse->Eval(wo, wi);
        ret += diffuse;
    }

    // specular
    specular = m_disney_specular_reflection->Eval(wo, wi);
    ret += specular;

    // clearcoat
    if (m_clearcoat > 0.0_r) {
        clearcoat = m_disney_clearcoat->Eval(wo, wi);
        ret += clearcoat;
    }

    return ret;
}

BSDFSampleInfo DisneyBSDF::SampleBSDF(const vec3 &wo_w, const vec3 &samples) const
{
    vec3 wo = WorldToShading(wo_w);

    real roulette = samples.z;
    vec3 wi = black;

    // transmission from interior to exterior
    if (wo.z < 0) {
        wi = m_disney_specular_transmission->Sample(wo, samples);
        return SampleInfoFromWoWi(wo, wi);
    }

    if (roulette < m_w_diffuse_refl) {
        wi = m_disney_diffuse->Sample(wo, samples);
    } else if (roulette < m_w_specular_refl) {
        wi = m_disney_specular_reflection->Sample(wo, samples);
    } else if (roulette < m_clearcoat) {
        wi = m_disney_clearcoat->Sample(wo, samples);
    } else {
        wi = m_disney_specular_transmission->Sample(wo, samples);
    }

    return SampleInfoFromWoWi(wo, wi);
}

BSDFSampleInfo DisneyBSDF::SampleInfoFromWoWi(const vec3 &wo, const vec3 &wi) const
{
    if (glm::length(wi) < eps_pdf) {
        return {black, black, 0, false};
    }

    real pdf = PdfLocal(wo, wi);
    vec3 f = CalFuncLocal(wo, wi);
    return {f, ShadingToWorld(wi), pdf, false};
}

real DisneyBSDF::PdfLocal(const vec3 &wo, const vec3 &wi) const
{
    real total_pdf = 0.0_r;

    // transmission from interior to exterior
    if (wo.z < 0) {
        return m_disney_specular_transmission->Pdf(wo, wi);
    }

    if (m_w_diffuse_refl > 0.0_r)
        total_pdf += m_w_diffuse_refl * m_disney_diffuse->Pdf(wo, wi);

    if (m_w_specular_refl > 0.0_r)
        total_pdf += m_w_specular_refl * m_disney_specular_reflection->Pdf(wo, wi);

    if (m_w_clearcoat > 0.0_r)
        total_pdf += m_w_clearcoat * m_disney_clearcoat->Pdf(wo, wi);

    if (m_w_specular_trans > 0.0_r)
        total_pdf += m_w_specular_trans * m_disney_specular_transmission->Pdf(wo, wi);

    return total_pdf;
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
    SP<Texture> m_specTransRoughness;
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
        SP<Texture> specTransRoughness,
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
        m_specTransRoughness = specTransRoughness;
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
        const real specTransRoughness = m_specTransRoughness->Sample(hit_point.uv).x;
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
                                            specTransRoughness,
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
        SP<Texture> specTransRoughness,
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
            specTransRoughness,
            diffTrans,
            flatness,
            ior,
            thin);
}

FM_ENGINE_END