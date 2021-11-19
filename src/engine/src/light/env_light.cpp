#include <engine/core/light.h>
#include "env_importance_sampler.h"

FM_ENGINE_BEGIN

void EnvLight::CalMeanRadiance()
{
    mean_radiance = black;

    const size_t width = texture->width();
    const size_t height = texture->height();

    const real du = 1.0_r / width;
    const real dv = 1.0_r / height;

    for (size_t j = 0; j < height; j++) {
        const real v0 = real(j) / height;
        const real sin_theta = std::sin(PI * (v0 + 0.5_r * dv));
        const real area = du * sin_theta;

        for (size_t i = 0; i < width; i++) {
            const real u0 = real(i) / width;
            mean_radiance += PI * area * texture->Sample({u0 + 0.5_r * du, v0 + 0.5_r * dv});
        }
    }
}

EnvLight::EnvLight(const SP<const Texture>& texture_)
{
    texture = texture_;
    CalMeanRadiance();
    sampler = MakeSP<EnvImportanceSampler>(texture_);
}

LightSampleInfo EnvLight::Sample(const HitPoint& hit_point, const vec3& sample) const
{
    const auto info = sampler->Sample(sample);
    const auto radiance = GetRadiance({}, {}, {}, -info.dir);
    return {hit_point.pos, hit_point.pos + info.dir * world_radius, -info.dir, radiance, info.pdf, REAL_MAX};
}

vec3 EnvLight::GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const
{
    const auto sphere_uv = GetSphericalUV(-light_to_shd);
    return texture->Sample({sphere_uv.x, sphere_uv.y});
}

real EnvLight::Pdf(const vec3& dir) const
{
    return sampler->Pdf(dir);
}

bool EnvLight::IsDelta() const
{
    return false;
}

real EnvLight::Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const
{
    return 0;
}

SP<EnvLight> CreateEnvLight(const SP<const Texture>& texture_)
{
    return MakeSP<EnvLight>(texture_);
}

FM_ENGINE_END
