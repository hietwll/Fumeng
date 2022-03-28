#include <engine/core/texture.h>
#include <engine/core/utils.h>
#include <engine/core/fresnel.h>
#include <engine/create/texture.h>
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
        const real cos_d = glm::dot(wi, wh.z > 0 ? wh : -wh);

        DisneyDielectricFresnel dielectric_fresnel(m_p->m_ior);
        const vec3 f_dielectric = m_p->m_cspec0 * dielectric_fresnel.CalFr(cos_d);
        const vec3 f_conduct = mat_func::SchlickFresnel(m_p->m_cspec0, std::abs(cos_d));
        const vec3 f = Lerp(m_p->m_specular * f_dielectric, f_conduct, m_p->m_metallic);

        // microfacet normal distribution
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);

        // geometry mask
        const real g = mat_func::SmithGGXAnisotropic(wo, wi, m_p->m_alpha_x, m_p->m_alpha_y);

        return f * d * g / std::abs(4.0_r * cos_i * cos_o);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        vec3 wh = mat_func::SampleGTR2(m_p->m_alpha_x, m_p->m_alpha_y, samples);
        if (IsReflection(wo, wh)) {
            Flip(wh);
        }
        vec3 wi = glm::normalize(2.0_r * glm::dot(wo, wh) * wh - wo);
        return IsReflection(wo, wi) ? wi : Flip(wi);
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return 0.0_r;
        }

        const vec3 wh = glm::normalize(wo + wi); // half vector
        const real cos_d = glm::dot(wi, wh.z > 0 ? wh : -wh);
        // microfacet normal distribution
        const real d = mat_func::GTR2Anisotropic(wh, m_p->m_alpha_x, m_p->m_alpha_y);
        return  std::abs(AbsCosDir(wh) * d / (4.0_r * cos_d));
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
        const real cos_o = std::abs(CosDir(wo)); // view dir
        const real cos_i = std::abs(CosDir(wi)); // light dir
        const real cos_d = std::abs(glm::dot(wi, wh));

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

        real wt = m_p->m_diffuse_weight;

        if (m_p->m_thin) {
            wt *= ((1.0_r - m_p->m_flatness) * (1.0_r - m_p->m_diffTrans));
        }

        return wt * f_lambert * (1.0_r - 0.5_r * fl) * (1.0_r - 0.5_r * fv) + f_retro + f_sheen;
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        // ignore normal, always sample in the same hemisphere
        vec3 wi = CosineWeightedHemiSphere(samples);
        return IsReflection(wo, wi) > 0 ? wi : Flip(wi);
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
        const real cos_o = std::abs(CosDir(wo)); // view dir
        const real cos_i = std::abs(CosDir(wi)); // light dir
        const real cos_d = std::abs(glm::dot(wi, wh));

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
        vec3 wh = mat_func::SampleGTR1(m_p->m_clearcoat_roughness, samples);
        if (IsReflection(wo, wh)) {
            Flip(wh);
        }
        vec3 wi = glm::normalize(2.0_r * glm::dot(wo, wh) * wh - wo);
        return IsReflection(wo, wi) ? wi : Flip(wi);
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
        return std::abs(AbsCosDir(wh) * d / (4.0_r * cos_d));
    }
};

class DisneyMicrofacetTransmission : public BaseBXDF
{
private:
    real m_alpha_x;
    real m_alpha_y;
    vec3 m_color;

public:
    DisneyMicrofacetTransmission(const DisneyBSDF* disneyBSDF, real alpha_x, real alpha_y, vec3 color)
    : BaseBXDF(disneyBSDF), m_alpha_x(alpha_x), m_alpha_y(alpha_y), m_color(color)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsRefraction(wo, wi)) {
            return black;
        }

        /**
         * refraction is not reciprocal, in/out direction influences relative IOR
         * eta is IOR_light / IOR_view
         * wo is view direction, wi is light direction
         */
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
        const real d = mat_func::GTR2Anisotropic(wh, m_alpha_x, m_alpha_y);

        // geometry mask
        const real g = mat_func::SmithGGXAnisotropic(wo, wi, m_alpha_x, m_alpha_y);

        const real den = eta * cos_d_i + cos_d_o;

        // Burley 2015, Eq(2)
        const real f_t = (1.0_r - f) * d * g * cos_d_o * cos_d_i / (cos_i * cos_o * den * den);

        return m_p->m_specTrans * (1.0_r - m_p->m_metallic) * m_color * std::abs(f_t);
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        vec3 wh = mat_func::SampleGTR2(m_p->m_trans_alpha_x, m_p->m_trans_alpha_y, samples);

        // eta_r for RefractDir should be incident IOR (+n) / transmitted IOR (-n)
        const real eta_r = wo.z > 0 ? m_p->m_ior_r : m_p->m_ior;

        // normal for RefractDir should be in same hemisphere with wo
        vec3 norm = IsReflection(wo, wh) > 0 ? wh : Flip(wh);

        // get transmitted direction
        const auto wt = mat_func::RefractDir(wo, norm, eta_r);

        // total internal reflection, should be handled in Inner Reflection
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
public:
    DisneyFakeSS(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsReflection(wo, wi)) {
            return black;
        }

        const vec3 wh = glm::normalize(wo + wi);
        const real cos_o = std::abs(CosDir(wo)); // view dir
        const real cos_i = std::abs(CosDir(wi)); // light dir
        const real cos_d = std::abs(glm::dot(wi, wh));

        const real fss90 = cos_d * cos_d * m_p->m_roughness;
        const real fo = mat_func::PowerFive(1.0_r - cos_o);
        const real fi = mat_func::PowerFive(1.0_r - cos_i);
        const real fss = Lerp(fo, 1.0_r, fss90) * Lerp(fi, 1.0_r, fss90);
        const real ss = 1.25_r * (fss * (1.0_r / (cos_o + cos_i) - 0.5_r) + 0.5_r);
        const real weight = m_p->m_diffuse_weight * m_p->m_flatness * (1.0_r - m_p->m_diffTrans);
        return weight * m_p->m_basecolor * InvPI * ss;
    }
};

class DisneyLambertianTransmission : public BaseBXDF
{
public:
    DisneyLambertianTransmission(const DisneyBSDF* disneyBSDF)
    : BaseBXDF(disneyBSDF)
    {
    }

    vec3 Eval(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsRefraction(wo, wi)) {
            return black;
        }

        return m_p->m_diffTrans * m_p->m_basecolor;
    }

    vec3 Sample(const vec3& wo, const vec2& samples) const override
    {
        vec3 wi = CosineWeightedHemiSphere(samples);
        return IsReflection(wo, wi) ? Flip(wi) : wi;
    }

    real Pdf(const vec3 &wo, const vec3 &wi) const override
    {
        if (!IsRefraction(wo, wi)) {
            return 0.0_r;
        }
        return std::abs(wi.z) * InvPI;
    }
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
    m_diffuse_weight = (1.0_r - m_metallic) * (1.0_r - m_specTrans);

    const real aspect = m_anisotropic > 0 ?
            std::sqrt(1 - real(0.9) * m_anisotropic) : real(1);
    m_alpha_x = std::max(real(0.001), m_roughness * m_roughness / aspect);
    m_alpha_y = std::max(real(0.001), m_roughness * m_roughness * aspect);

    m_trans_alpha_x = std::max(real(0.001), m_specTransRoughness * m_specTransRoughness / aspect);
    m_trans_alpha_y = std::max(real(0.001), m_specTransRoughness * m_specTransRoughness * aspect);


    // weight
    const real basecolor_lum = RGBToLuminance(m_basecolor);
    const real cspec0_lum = RGBToLuminance(m_cspec0);
    m_w_diffuse_refl = basecolor_lum * m_diffuse_weight * (m_thin ? (1.0_r - m_diffTrans) : 1.0_r);
    m_w_specular_refl = cspec0_lum * 2.0_r;
    m_w_clearcoat = m_clearcoat;
    m_w_specular_trans = basecolor_lum * (1.0_r - m_metallic) * specTrans;
    m_w_diffuse_trans = m_thin ? basecolor_lum * m_diffuse_weight * m_diffTrans : 0.0_r;

    const real w_sum_inv = 1.0_r / (m_w_diffuse_refl + m_w_specular_refl
            + m_w_clearcoat + m_w_specular_trans + m_w_diffuse_trans);

    m_w_diffuse_refl *= w_sum_inv;
    m_w_specular_refl *= w_sum_inv;
    m_w_clearcoat *= w_sum_inv;
    m_w_specular_trans *= w_sum_inv;
    m_w_diffuse_trans *= w_sum_inv;

    m_c_diffuse_refl = m_w_diffuse_refl;
    m_c_specular_refl = m_w_specular_refl + m_c_diffuse_refl;
    m_c_clearcoat = m_w_clearcoat + m_c_specular_refl;
    m_c_specular_trans = m_w_specular_trans + m_c_clearcoat;
    m_c_diff_trans = m_w_diffuse_trans + m_c_specular_trans;

    // submodels
    m_disney_specular_reflection = MakeUP<DisneySpecularReflection>(this);
    m_disney_diffuse = MakeUP<DisneyDiffuse>(this);
    m_disney_clearcoat = MakeUP<DisneyClearCoat>(this);
    m_disney_specular_transmission =
            MakeUP<DisneyMicrofacetTransmission>(this, m_trans_alpha_x, m_trans_alpha_y, m_basecolor);

    if (m_thin) {
        const real roughness_scale = (0.65_r * m_ior - 0.35_r) * m_specTransRoughness;
        const real alpha_xs = std::max(real(0.001), roughness_scale * roughness_scale / aspect);
        const real alpha_ys = std::max(real(0.001), roughness_scale * roughness_scale * aspect);
        m_disney_rough_transmission =
                MakeUP<DisneyMicrofacetTransmission>(this, alpha_xs, alpha_ys, glm::sqrt(m_basecolor));

        m_disney_lambert_transmission = MakeUP<DisneyLambertianTransmission>(this);
        m_disney_fake_ss = MakeUP<DisneyFakeSS>(this);
    }

    m_opaque = !(m_thin || m_specTrans > 0.0_r);
}

vec3 DisneyBSDF::CalFuncLocal(const vec3 &wo, const vec3 &wi) const
{
    vec3 ret = black;

    // handle transmission
    if (wo.z * wi.z < 0) {
        if (m_thin) {
            ret += m_disney_rough_transmission->Eval(wo, wi);
            ret += m_disney_lambert_transmission->Eval(wo, wi);
        } else {
            ret += m_disney_specular_transmission->Eval(wo, wi);
        }
        return ret;
    }

    // diffuse
    if (m_metallic < 1.0_r && m_specTrans < 1.0_r) {
        if (m_thin && m_flatness > 0.0_r) {
            ret += m_disney_fake_ss->Eval(wo, wi);
        }
        ret += m_disney_diffuse->Eval(wo, wi);
    }

    // specular
    ret += m_disney_specular_reflection->Eval(wo, wi);

    // clearcoat
    if (m_clearcoat > 0.0_r) {
        ret += m_disney_clearcoat->Eval(wo, wi);
    }

    return ret;
}

BSDFSampleInfo DisneyBSDF::SampleBSDF(const vec3 &wo_w, const vec3 &samples) const
{
    vec3 wo = WorldToShading(wo_w);

    real roulette = samples.z;
    vec3 wi = black;

    wi = RouletteSample(wo, roulette, samples);
    return SampleInfoFromWoWi(wo, wi);
}

vec3 DisneyBSDF::RouletteSample(const vec3 &wo, real roulette, const vec3& samples) const
{
    if (roulette <= m_c_diffuse_refl) {
        return m_disney_diffuse->Sample(wo, samples);
    } else if (roulette <= m_c_specular_refl) {
        return m_disney_specular_reflection->Sample(wo, samples);
    } else if (roulette <= m_c_clearcoat) {
        return m_disney_clearcoat->Sample(wo, samples);
    } else if (roulette <= m_c_specular_trans){
        return m_thin ? m_disney_rough_transmission->Sample(wo, samples) :
                m_disney_specular_transmission->Sample(wo, samples);
    } else if (roulette <= m_c_diff_trans) {
        return m_disney_lambert_transmission->Sample(wo, samples);
    } else {
        // fall back to diffuse at corner case for single precision
        return m_disney_diffuse->Sample(wo, samples);
    }
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

    if (m_w_diffuse_refl > 0.0_r)
        total_pdf += m_w_diffuse_refl * m_disney_diffuse->Pdf(wo, wi);

    if (m_w_specular_refl > 0.0_r)
        total_pdf += m_w_specular_refl * m_disney_specular_reflection->Pdf(wo, wi);

    if (m_w_clearcoat > 0.0_r)
        total_pdf += m_w_clearcoat * m_disney_clearcoat->Pdf(wo, wi);

    if (m_w_specular_trans > 0.0_r)
        total_pdf += m_w_specular_trans * (m_thin ? m_disney_rough_transmission->Pdf(wo, wi) :
                m_disney_specular_transmission->Pdf(wo, wi));

    if (m_w_diffuse_trans > 0.0_r)
        total_pdf += m_w_diffuse_trans * m_disney_lambert_transmission->Pdf(wo, wi);

    return total_pdf;
}

vec3 DisneyBSDF::GetAlbedo() const
{
    return m_basecolor;
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
    DisneyMaterial(const DisneyConfig& config)
    {
        m_basecolor = CreateTexture(config.basecolor);
        m_metallic = CreateTexture(config.metallic);
        m_specular = CreateTexture(config.specular);
        m_specularTint = CreateTexture(config.specularTint);
        m_roughness = CreateTexture(config.roughness);
        m_anisotropic = CreateTexture(config.anisotropic);
        m_sheen = CreateTexture(config.sheen);
        m_sheenTint = CreateTexture(config.sheenTint);
        m_clearcoat = CreateTexture(config.clearcoat);
        m_clearcoatGloss = CreateTexture(config.clearcoatGloss);
        m_specTrans = CreateTexture(config.specTrans);
        m_specTransRoughness = CreateTexture(config.specTransRoughness);
        m_diffTrans = CreateTexture(config.diffTrans);
        m_flatness = CreateTexture(config.flatness);
        m_ior = CreateTexture(config.ior);
        m_thin = config.thin;
    }

    void CreateBSDF(HitPoint &hit_point) const override
    {
        const vec3 basecolor = m_basecolor->Sample(hit_point.uv);
        const real metallic = m_metallic->Sample(hit_point.uv).z;
        const real specular = m_specular->Sample(hit_point.uv).x;
        const real specularTint = m_specularTint->Sample(hit_point.uv).x;
        const real roughness = m_roughness->Sample(hit_point.uv).y;
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

SP<Material> CreateDisneyMaterial(const DisneyConfig& config)
{
    return MakeSP<DisneyMaterial>(config);
}

FM_ENGINE_END