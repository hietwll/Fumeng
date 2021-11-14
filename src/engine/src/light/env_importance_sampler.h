#ifndef FUMENG_ENV_IMPORTANCE_SAMPLER_H
#define FUMENG_ENV_IMPORTANCE_SAMPLER_H

#include <engine/common.h>
#include <engine/core/texture.h>
#include <engine/core/distribution.h>

FM_ENGINE_BEGIN

class EnvImportanceSampler
{
private:
    Distribution2D::vector2d lum;

public:
    EnvImportanceSampler(const SP<const Texture>& texture)
    {
        const size_t width = texture->width();
        const size_t height = texture->height();

        lum.resize(width * height);

        // compute scalar luminance
        const real du = 1.0_r / width;
        const real dv = 1.0_r / height;
        real total_lum = 0.0_r;

        for (size_t j = 0; j < height; j++) {
            const real v0 = real(j) / height;
            const real sin_theta = std::sin(PI * (v0 + 0.5_r * dv));

            for (size_t i = 0; i < width; i++) {
                const real u0 = real(i) / width;
                const vec3 radiance = texture->Sample({u0 + 0.5_r * du, v0 + 0.5_r * dv});
                const real luminance = RGBToLuminance(radiance) * sin_theta;
                lum[i + j * width] = luminance;
                total_lum += luminance;
            }
        }
    }
};

FM_ENGINE_END

#endif
