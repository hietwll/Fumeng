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
inline vec3 CosineWeightedHemiSphere(const vec2& samples)
{
    real zz;
    vec2 proj = UniformDisk(samples);
    zz = 1.0_r - proj.x * proj.x - proj.y * proj.y;
    if(zz < 0.0_r) {
        zz = 0.0_r;
    }
    return {proj.x, proj.y, std::sqrt(zz)};
}

/*
 * Uniformly sample a point on a sphere
 */
inline vec3 UniformSphere(const vec2& samples) {
    real z = 1.0_r - 2.0_r * samples.x;
    real r = std::sqrt(std::max(0.0_r, 1.0_r - z * z));
    real phi = 2.0_r * PI * samples.y;
    return {r * std::cos(phi), r * std::sin(phi), z};
}

/*
 * Uniformly sample a point on cone with angle theta
 * pdf is w.r.t area here, cone is assumed to point to (0, 0, 1)
 */
inline void UniformCone(const real& cos_t_min, const vec2& samples,
                        vec3* pos, real* pdf)
{
    // cos_t_max = 1.0
    const real cos_t = cos_t_min + samples.x * (1.0_r - cos_t_min);
    const real sin_t = std::sqrt(std::max(0.0_r, 1.0_r - cos_t * cos_t));
    const real phi = 2.0_r * PI * samples.y;
    pos->x = std::cos(phi) * sin_t; // radius is 1.0
    pos->y = std::sin(phi) * sin_t;
    pos->z = cos_t;
    *pdf = 1.0_r / (2.0_r * PI * (1.0_r - cos_t_min)); // hemisphere if cos_t_min = 0
    return;
}

inline real SquareSum(const vec3& vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

inline real Clamp(real v, const real& low, const real& up)
{
    return v > up ? up : (v < low ? low : v);
}

inline void CreateCoordSys(const vec3& v1, vec3* v2, vec3* v3)
{
    if (std::abs(v1.x) > std::abs(v1.y))
    {
        *v2 = glm::normalize(vec3(-v1.z, 0.0_r, v1.x));
    } else {
        *v2 = glm::normalize(vec3(0.0_r, v1.z, -v1.y));
    }

    *v3 = glm::cross(v1, *v2);
}

inline bool IsBlack(const vec3& color)
{
    if(color.x == 0.0_r && color.y == 0.0_r && color.z == 0.0_r) {
        return true;
    }

    if(color.x < 0.0_r || color.y < 0.0_r || color.z < 0.0_r) {
        return true;
    }

    return false;
}

inline real DegToRad(real deg)
{
    return deg / 180.0_r * PI;
}

FM_ENGINE_END

#endif