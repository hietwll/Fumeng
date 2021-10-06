#ifndef FM_ENGINE_LIGHT_H
#define FM_ENGINE_LIGHT_H

#include <engine/common.h>

FM_ENGINE_BEGIN

struct LightSampleInfo
{
    vec3 ref_pos; // position of sampled point in world space
    vec3 light_pos; // position of sampled light in world space
    vec3 wi_w; // direction of sampled light in world space
    vec3 radiance; // radiance of sampled light
    real pdf = 0.0_r; // pdf of the sampled light w.r.t. solid angle

    LightSampleInfo(const vec3& r_pos, const vec3& l_pos, const vec3& wi_, const vec3& radiance_, real pdf_)
    : ref_pos(r_pos), light_pos(l_pos), wi_w(wi_), radiance(radiance_), pdf(pdf_)
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
        const real dist = ref_to_light.length();
        const vec3 wi = glm::normalize(ref_to_light);
        const real pdf_angle = pdf_area * dist * dist / AbsDot(geom_sample.ng, -wi);

        return {hit_point.pos, geom_sample.pos, wi, radiance, pdf_angle};
    }

    bool IsDelta() const override
    {
        return false;
    }
};


FM_ENGINE_END

#endif