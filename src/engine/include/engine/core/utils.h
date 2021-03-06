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
    return glm::normalize(vec3(proj.x, proj.y, std::sqrt(zz)));
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

template<typename T>
inline T Clamp(T v, const T low, const T up)
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

inline real AbsDot(const vec3& v1, const vec3& v2)
{
    return std::abs(glm::dot(v1, v2));
}

inline real PowerHeuristic(real f, real g)
{
    return (f * f) / (f * f + g * g);
}

inline void Clamp(vec3& color, real low = 0.0_r, real up = 1.0_r)
{
    color.x = Clamp(color.x, low, up);
    color.y = Clamp(color.y, low, up);
    color.z = Clamp(color.z, low, up);
}

inline real SrgbToLinear(real color)
{
    if (color <= 0.04045_r)
        return color * 1.0_r / 12.92_r;
    return std::pow((color + 0.055_r) * 1.0_r / 1.055_r, (real)2.4_r);
}

inline real LinearToSrgb(real color)
{
    if (color <= 0.0031308_r)
        return 12.92_r * color;
    return 1.055f * std::pow(color, (real)(1.0_r / 2.4_r)) - 0.055_r;
}

inline uint8_t RealToUInt8(real color) {
    color = Clamp(color, 0.0_r, 1.0_r);
    return static_cast<uint8_t>(round(255.0_r * color));
}

/**
 * Convert linear RGB to luminance
 * @param color RGB in linear space
 * @return
 */
inline real RGBToLuminance(const vec3& color)
{
    return 0.2126_r * color.x + 0.7152_r * color.y + 0.0722_r * color.z;
}

inline real GetSphericalTheta(const vec3& dir)
{
    return std::acos(Clamp(dir.z, -1.0_r, 1.0_r));
}

inline real GetSphericalPhi(const vec3& dir)
{
    const real p = std::atan2(dir.y, dir.x);
    return (p < 0) ? (p + 2.0_r * PI) : p;
}

inline vec3 GetSphericalUV(const vec3& dir)
{
    // get theta from dir
    const real theta = GetSphericalTheta(dir);

    // get phi from dir
    const real phi = GetSphericalPhi(dir);

    // get u,v
    const real u = phi * InvPI * 0.5_r;
    const real v = theta * InvPI;

    return {u, v, theta};
}

template<typename T>
inline auto Lerp(const T& start, const T& end, real amt)
{
    return (1.0_r - amt) * start + amt * end;
}

inline vec3 ToTint(const vec3& color)
{
    const real luminance = RGBToLuminance(color);
    return luminance > 0.0_r ? color * (1.0_r / luminance) : white;
}

/*
 * Calculate the cos_i between wi and normal (0, 0 ,1) in shading space.
 * wi \cdot n = |wi||n| cos_i
 */
inline real CosDir(const vec3& wi)
{
    return glm::normalize(wi).z;
}

inline real AbsCosDir(const vec3& wi)
{
    return std::abs(CosDir(wi));
}

inline vec3 MinVec(const vec3& lhs, const vec3& rhs)
{
    return vec3(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z));
}

inline vec3 MaxVec(const vec3& lhs, const vec3& rhs)
{
    return vec3(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z));
}

inline real MaxElem(const vec3& v)
{
    return v.x > v.y ? (v.x > v.z ? v.x : v.z) : (v.y > v.z ? v.y : v.z);
}

inline real MinElem(const vec3& v)
{
    return v.x < v.y ? (v.x < v.z ? v.x : v.z) : (v.y < v.z ? v.y : v.z);
}

FM_ENGINE_END

#endif