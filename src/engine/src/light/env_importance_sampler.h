#ifndef FUMENG_ENV_IMPORTANCE_SAMPLER_H
#define FUMENG_ENV_IMPORTANCE_SAMPLER_H

#include <engine/common.h>
#include <engine/core/texture.h>
#include <engine/core/distribution.h>

FM_ENGINE_BEGIN

struct EnvImportanceSamplerInfo
{
    vec3 dir; // direction from shading point to light
    real pdf; // pdf w.r.t solid angle

    EnvImportanceSamplerInfo(const vec3 dir_, real pdf_):
    dir(dir_), pdf(pdf_)
    {
    }
};

class EnvImportanceSampler
{
private:
    UP<Distribution2D> distribution2D;
    Distribution2D::vector2d lum;

public:
    EnvImportanceSampler(const SP<const Texture>& texture)
    {
        const size_t width = texture->width();
        const size_t height = texture->height();

        lum.resize(height);
        for (size_t j = 0; j < height; ++j) {
            lum[j].resize(width);
        }

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
                lum[j][i] = luminance;
                total_lum += luminance;
            }
        }

        // it's important to normalize the pdf, sum of p(u,v) should be 1
        for (auto& x : lum) {
            for (auto& y : x) {
                y = y / total_lum * width * height;
            }
        }

        // init 2d distribution by luminance
        distribution2D = MakeUP<Distribution2D>(lum, width, height);
    }

    EnvImportanceSamplerInfo Sample(const vec3& sample) const
    {
        const auto sample_2d = distribution2D->Sample(sample);
        const real phi = sample_2d.val.x * 2.0_r * PI;
        const real theta = sample_2d.val.y * PI;
        const real sin_phi = std::sin(phi);
        const real cos_phi = std::cos(phi);
        const real sin_theta = std::sin(theta);
        const real cos_theta = std::cos(theta);

        if (sin_theta == 0) {
            return {black, 0.0_r};
        }

        const real pdf = sample_2d.pdf / (2.0_r * PI * PI * sin_theta);
        return {{sin_theta * cos_phi, sin_theta * sin_phi, cos_theta}, pdf};
    }

    real Pdf(const vec3& dir) const
    {
        const auto uv = GetSphericalUV(dir);

        const real sin_theta = std::sin(uv.z);
        if (sin_theta == 0) {
            return 0;
        }

        return distribution2D->Pdf({uv.x, uv.y}) / (2.0_r * PI * PI * sin_theta);
    }
};

FM_ENGINE_END

#endif
