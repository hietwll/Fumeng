#ifndef FM_ENGINE_BSDF_H
#define FM_ENGINE_BSDF_H

#include <engine/common.h>
#include <engine/core/hit_point.h>
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

enum BSDFType : uint8_t
{
    BSDF_DIFFUSE = 1 << 0,
    BSDF_GLOSSY  = 1 << 1,
    BSDF_SPECULAR = 1 << 2
};

struct BSDFSampleInfo
{
    vec3 f; // bsdf value
    vec3 wi_w; // sampled direction in world space
    real pdf = 0.0_r; // pdf
    bool is_delta = false; // if bsdf is delta function

    BSDFSampleInfo(vec3 value, vec3 dir, real density, bool delta) :
    f(value), wi_w(dir), pdf(density), is_delta(delta)
    {
    }
};

class BSDF
{
private:
    vec3 ng; // geometry normal, world space
    vec3 xs; // x axis of shading coordinate (primary tangent), world space
    vec3 ys; // y axis of shading coordinate (secondary tangent), world space
    vec3 zs; // z axis of shading coordinate (shading normal), world space

public:
    BSDF(const HitPoint& hit_point) :
    ng(hit_point.ng), zs(hit_point.ns), xs(glm::normalize(hit_point.ss)), ys(glm::cross(zs, xs))
    {
    };

    ~BSDF() = default;

    /*
     * Calculate the BRDF function in world space
     * _w means world space, need to transformed to shading space for cal
     */
    vec3 CalFunc(const vec3& wo_w, const vec3& wi_w) const
    {
        return CalFuncLocal(WorldToShading(wo_w), WorldToShading(wi_w));
    };

    /*
     * Calculate the BRDF function in shading space
     */
    virtual vec3 CalFuncLocal(const vec3& wo, const vec3& wi) const = 0;

    /*
     * Get the p.d.f. of given wi_w in world space
     */
    real Pdf(const vec3& wo_w, const vec3& wi_w) const
    {
        return PdfLocal(WorldToShading(wo_w), WorldToShading(wi_w));
    };

    /*
     * Get the p.d.f. of given wi in shading space
     * by default, we use cosine-weighted hemisphere sampling because the scattering equation has a
     * cosine term, we should not put too much cost for angle with cosine = 0.
     */
    virtual real PdfLocal(const vec3& wo, const vec3& wi) const
    {
        if(wo.z * wi.z < 0.0_r) {
            return 0.0_r;
        }
        return std::abs(wi.z) * InvPI; // the hemisphere integration of cosine is PI
    }

    /*
     * Sampling the BSDF
     * by default, we use cosine-weighted hemisphere sampling
     */
    virtual BSDFSampleInfo SampleBSDF(const vec3& wo_w, const vec2& samples) const
    {
        vec3 wo = WorldToShading(wo_w);
        vec3 wi = CosineWeightedHemiSphere(samples);
        if (wo.z < 0.0_r) {
            wi.z *= -1.0_r; // make sure they are in the same hemisphere
        }

        real pdf = PdfLocal(wo, wi);
        vec3 f = CalFuncLocal(wo, wi);
        return {f, ShadingToWorld(wi), pdf, false};
    };

    /*
     * World space to shading space
     */
    vec3 WorldToShading(const vec3& vec) const
    {
        return glm::normalize(
                vec3(glm::dot(vec, xs), glm::dot(vec, ys), glm::dot(vec, zs))
                );
    };

    /*
     * Shading space to world space
     */
    vec3 ShadingToWorld(const vec3& vec) const
    {
        return glm::normalize(
                vec3(xs.x * vec.x + ys.x * vec.y + zs.x * vec.z,
                    xs.y * vec.x + ys.y * vec.y + zs.y * vec.z,
                    xs.z * vec.x + ys.z * vec.y + zs.z * vec.z)
                );
    };

    /*
     * Calculate the cos_i between wi and normal (0, 0 ,1) in shading space.
     * wi \cdot n = |wi||n| cos_i
     */
    real CosDir(const vec3& wi) const
    {
        return glm::normalize(wi).z;
    }
};

FM_ENGINE_END

#endif