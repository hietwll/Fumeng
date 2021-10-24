#ifndef FM_ENGINE_LIGHT_H
#define FM_ENGINE_LIGHT_H

#include <engine/common.h>
#include <engine/core/hit_point.h>
#include <engine/core/geometry.h>
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

struct LightSampleInfo
{
    vec3 ref_pos; // position of sampled point in world space
    vec3 light_pos; // position of sampled light in world space
    vec3 wi_w; // direction of sampled light in world space
    vec3 radiance; // radiance of sampled light
    real pdf = 0.0_r; // pdf of the sampled light w.r.t. solid angle
    real dist = 0.0_r; // lenght of ref_pos - light_pos

    LightSampleInfo(const vec3& r_pos, const vec3& l_pos, const vec3& wi_,
                    const vec3& radiance_, real pdf_, real dist_)
    : ref_pos(r_pos), light_pos(l_pos), wi_w(wi_), radiance(radiance_), pdf(pdf_), dist(dist_)
    {
    }
};

class Light
{
public:

    virtual ~Light() = default;

    /**
     * sample light wi at ref
     */
    virtual LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const = 0;

    /**
     * is this a delta light (delta position or delta direction)
     */
    virtual bool IsDelta() const = 0;

    /**
     * get the pdf of given position
     */
    virtual real Pdf(const vec3& ref_pos, const vec3& pos, const vec3& nor, const vec3& light_to_out) const = 0;
};

class AreaLight : public Light
{
private:
    const Geometry* geometry = nullptr;
    vec3 radiance;

public:
    AreaLight(const Geometry* geometry_, const vec3& radiance_)
    : geometry(geometry_), radiance(radiance_)
    {

    }

    LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const override
    {
        real pdf_area;
        auto geom_sample = geometry->Sample(hit_point, &pdf_area, sample);

        const vec3 ref_to_light = geom_sample.pos - hit_point.pos;
        const real dist = glm::length(ref_to_light);

        if (dist < eps) {
            return {black, black, black, black, 0.0_r, 0.0_r};
        }

        const vec3 wi = glm::normalize(ref_to_light);
        const real pdf_solid_angle = pdf_area * dist * dist / AbsDot(geom_sample.ng, -wi);

        return {hit_point.pos, geom_sample.pos, wi, radiance, pdf_solid_angle, dist};
    }

    bool IsDelta() const override
    {
        return false;
    }

    vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const
    {
        return glm::dot(nor, light_to_shd) > 0 ? radiance : black;
    }

    real Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const override
    {
        if (glm::dot(light_nor, light_to_shd) <0) {
            return 0.0_r;
        }

        // get pdf w.r.t. area from geometry
        const real pdf_area = geometry->Pdf(shd_pos, light_pos);

        // get pdf w.r.t. solid angle
        const real dist = glm::length(shd_pos - light_pos);
        return pdf_area * dist * dist / AbsDot(light_nor, light_to_shd);
    }
};


FM_ENGINE_END

#endif