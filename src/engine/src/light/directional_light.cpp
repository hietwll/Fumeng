#include <engine/core/light.h>

FM_ENGINE_BEGIN

DirectionalLight::DirectionalLight(const vec3& radiance, const vec3& direction) :
m_radiance(radiance),
m_direction(direction)
{
}

LightSampleInfo DirectionalLight::Sample(const HitPoint& hit_point, const vec3& sample) const
{
    return {hit_point.pos, hit_point.pos - m_direction * m_world_radius, -m_direction, m_radiance, 1.0_r, REAL_MAX};
}

bool DirectionalLight::IsDelta() const
{
    return true;
}

vec3 DirectionalLight::GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const
{
    // there is no way to meet a dirac light when sampling bsdf. the only way to considering dirac light is to sample light
    return black;
}

real DirectionalLight::Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const
{
    // always return zero pdf for Delta light
    return 0.0_r;
}

SP<Light> CreateDirectionalLight(const vec3& radiance, const vec3& direction)
{
    return MakeSP<DirectionalLight>(radiance, direction);
}
FM_ENGINE_END
