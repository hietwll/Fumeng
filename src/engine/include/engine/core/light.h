#ifndef FM_ENGINE_LIGHT_H
#define FM_ENGINE_LIGHT_H

#include <engine/common.h>
#include <engine/core/hit_point.h>
#include <engine/core/geometry.h>
#include <engine/core/utils.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

struct LightSampleInfo
{
    vec3 ref_pos; // position of sampled point in world space
    vec3 light_pos; // position of sampled light in world space
    vec3 wi_w; // direction of sampled light in world space (shading point to light)
    vec3 radiance; // radiance of sampled light
    real pdf = 0.0_r; // pdf of the sampled light w.r.t. solid angle
    real dist = 0.0_r; // length of ref_pos - light_pos

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
     * Sample light wi at ref
     */
    virtual LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const = 0;

    /**
     * Is this a delta light (delta position or delta direction)
     */
    virtual bool IsDelta() const = 0;

    /**
     * Get the pdf of given shading position, returned pdf is w.r.t. solid angle
     */
    virtual real Pdf(const vec3& ref_pos, const vec3& pos, const vec3& nor, const vec3& light_to_out) const = 0;

    /**
     * Approximation of power. some algorithms may devote additional computational resources to lights making large contribution
     */
     virtual vec3 GetPower() const
     {
         return black;
     };

     /**
      * Get the radiance of the light
      *
      * @param pos position on the light source
      * @param nor normal of the light source at position
      * @param uv uv coordinate
      * @param light_to_shd direction vector from shading point to point on light source
      * @return radiance
      */
     virtual vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const = 0;
};

class AreaLight : public Light
{
private:
    const Geometry* m_geometry = nullptr;
    vec3 m_radiance;

public:
    AreaLight(const Geometry* geometry, const vec3& radiance);

    LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const override;

    bool IsDelta() const override;

    vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const override;

    real Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const override;
};

class DirectionalLight : public Light
{
private:
    vec3 m_radiance;
    vec3 m_direction; // light to shading point in world space
    // todo: calculate world_radius by bounding box size
    real m_world_radius = 1.0e6_r;

public:
    DirectionalLight(const vec3& radiance, const vec3& direction);

    LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const override;

    bool IsDelta() const override;

    vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const override;

    real Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const override;
};

class EnvImportanceSampler;

class EnvLight : public Light
{
private:
    SP<const Texture> m_texture;
    Transform light_to_world, world_to_light;
    SP<const EnvImportanceSampler> m_sampler;
    vec3 m_mean_radiance; // power
    // todo: calculate world_radius by bounding box size
    real m_world_radius = 1.0e6_r;
    real m_factor;

    void CalMeanRadiance();

public:
    EnvLight(const SP<const Texture>& texture, const vec3& rotation, real factor);
    LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const override;
    vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const override;
    real Pdf(const vec3& dir) const;
    bool IsDelta() const override;
    real Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const override;
};

FM_ENGINE_END

#endif