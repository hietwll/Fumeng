#ifndef FM_ENGINE_UTILS_H
#define FM_ENGINE_UTILS_H

#include <engine/common.h>

FM_ENGINE_BEGIN

/*
 * Uniformly sample a disk based on r = sqrt(sample) and theta = 2PI(sample)
 */
inline vec2 UniformDisk(const vec2& samples)
{
    real r = std::sqrt(samples.x);
    real theta = 2.0 * PI * samples.y;
    return {r * std::cos(theta), r * std::sin(theta)};
}

/*
 * Sample a direction based on cosine-weighted p.d.f. over hemisphere
 * Use Malley’s method: uniformly sample a disk and project point to
 * hemisphere
 */
inline vec3 CosineWeightedHemisphere(const vec2& samples)
{
    real z;
    real zz;
    vec2 proj = UniformDisk(samples);
    zz = 1.0 - proj.x * proj.x - proj.y * proj.y;
    if(zz < 0.0) {
        zz = 0.0;
    }
    return {proj.x, proj.y, std::sqrt(zz)};
}

FM_ENGINE_END

#endif