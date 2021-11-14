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
    real radius = 1.0_r;

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

    }
};

FM_ENGINE_END
