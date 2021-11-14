#include <engine/core/light.h>
#include <engine/core/texture.h>
#include "env_importance_sampler.h"

FM_ENGINE_BEGIN

class EnvLight : public Light
{
private:
    SP<const Texture> texture;
    Transform light_to_world;
    SP<const EnvImportanceSampler> sampler;
    vec3 mean_radiance; // power
    // todo: calculate world_radius by bounding box size
    real world_radius = 1.0e6_r;

    void CalMeanRadiance()
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

public:
    EnvLight(const SP<const Texture>& texture_)
    {
        texture = texture_;
        CalMeanRadiance();
        sampler = MakeSP<EnvImportanceSampler>(texture_);
    }

    LightSampleInfo Sample(const HitPoint& hit_point, const vec3& sample) const override
    {
        const auto info = sampler->Sample(sample);
        const auto radiance = GetRadiance({}, {}, {}, -info.dir);
        return {hit_point.pos, hit_point.pos + info.dir * world_radius, -info.dir, radiance, info.pdf, REAL_MAX};
    }

    vec3 GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const override
    {
        const auto sphere_uv = GetSphericalUV(-light_to_shd);
        return texture->Sample({sphere_uv.x, sphere_uv.y});
    }

    real Pdf(const vec3& dir) const
    {
        return sampler->Pdf(dir);
    }
};

FM_ENGINE_END
