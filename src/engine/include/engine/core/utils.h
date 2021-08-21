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
    real theta = 2.0_r * PI * samples.y;
    return {r * std::cos(theta), r * std::sin(theta)};
}

/*
 * Sample a direction based on cosine-weighted p.d.f. over hemisphere
 * Use Malley’s method: uniformly sample a disk and project point to
 * hemisphere
 */
inline vec3 CosineWeightedHemisphere(const vec2& samples)
{
    real zz;
    vec2 proj = UniformDisk(samples);
    zz = 1.0_r - proj.x * proj.x - proj.y * proj.y;
    if(zz < 0.0_r) {
        zz = 0.0_r;
    }
    return {proj.x, proj.y, std::sqrt(zz)};
}

inline real SquareSum(const vec3& vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

inline real Clamp(real v, const real& low, const real& up)
{
    return v > up ? up : (v < low ? low : v);
}

FM_ENGINE_END

#endif