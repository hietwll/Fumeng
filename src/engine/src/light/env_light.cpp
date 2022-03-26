#include <engine/core/light.h>
#include "env_importance_sampler.h"

FM_ENGINE_BEGIN

void EnvLight::CalMeanRadiance()
{
    m_mean_radiance = black;

    const size_t width = m_texture->width();
    const size_t height = m_texture->height();

    const real du = 1.0_r / width;
    const real dv = 1.0_r / height;

    for (size_t j = 0; j < height; j++) {
        const real v0 = real(j) / height;
        const real sin_theta = std::sin(PI * (v0 + 0.5_r * dv));
        const real area = du * sin_theta;

        for (size_t i = 0; i < width; i++) {
            const real u0 = real(i) / width;
            m_mean_radiance += m_factor * PI * area * m_texture->Sample({u0 + 0.5_r * du, v0 + 0.5_r * dv});
        }
    }
}

EnvLight::EnvLight(const SP<const Texture>& texture, const vec3& rotation, real factor) :
light_to_world(Transform(black, rotation, white)),
m_factor(factor),
world_to_light(light_to_world.InvTransform())
{
    m_texture = texture;
    CalMeanRadiance();
    m_sampler = MakeSP<EnvImportanceSampler>(texture);
}

LightSampleInfo EnvLight::Sample(const HitPoint& hit_point, const vec3& sample) const
{
    const auto info = m_sampler->Sample(sample);
    const auto dir_world = light_to_world.ApplyToVec3(info.dir);
    const auto radiance = GetRadiance({}, {}, {}, -dir_world);
    return {hit_point.pos, hit_point.pos + dir_world * m_world_radius, dir_world, radiance, info.pdf, REAL_MAX};
}

/**
 * @param light_to_shd direction of light to shade in world space
 */
vec3 EnvLight::GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const
{
    const auto sphere_uv = GetSphericalUV(world_to_light.ApplyToVec3(-light_to_shd));
    return m_factor * m_texture->Sample({sphere_uv.x, sphere_uv.y});
}

/*
 * direction should be shading point to light (in world space)
 */
real EnvLight::Pdf(const vec3& dir) const
{
    return m_sampler->Pdf(world_to_light.ApplyToVec3(dir));
}

bool EnvLight::IsDelta() const
{
    return false;
}

real EnvLight::Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const
{
    return m_sampler->Pdf(world_to_light.ApplyToVec3(-light_to_shd));
}

SP<EnvLight> CreateEnvLight(const SP<const Texture>& texture, const vec3& rotation, real factor)
{
    return MakeSP<EnvLight>(texture, rotation, factor);
}

FM_ENGINE_END
