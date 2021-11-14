#ifndef FM_ENGINE_COMMON_H
#define FM_ENGINE_COMMON_H

#define GLM_FORCE_LEFT_HANDED

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <spdlog/spdlog.h>

#define FM_ENGINE_BEGIN namespace fumeng::engine {
#define FM_ENGINE_END }

FM_ENGINE_BEGIN

#ifdef USE_FLOAT
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using real = float;
#else
using vec2 = glm::dvec2;
using vec3 = glm::dvec3;
using vec4 = glm::dvec4;
using mat4 = glm::dmat4;
using real = double;
#endif

using dreal = double;

real constexpr operator"" _r(long double v) {
    return real(v);
}

real constexpr operator"" _r(unsigned long long v) {
    return real(v);
}

dreal constexpr operator"" _d(long double v) {
    return dreal(v);
}

dreal constexpr operator"" _d(unsigned long long v) {
    return dreal(v);
}

constexpr real REAL_MAX = std::numeric_limits<real>::max();
constexpr real REAL_MIN = std::numeric_limits<real>::min();

const real PI = (real) 3.14159265354_d;
const real InvPI = (real) (1.0_d / PI);
const real eps = 1e-4_r;
const real eps_pdf = 1e-6_r;

// forward declaration
class Ray;
class Scene;
class Sampler;
class Camera;
class Transform;
class HitPoint;
class BSDF;
class Geometry;
class Material;
class RenderObject;
class Aggregate;
class Light;
class Image;
class Sphere;

// color
const vec3 red = vec3(1.0_r, 0.0_r, 0.0_r);
const vec3 green = vec3(0.0_r, 1.0_r, 0.0_r);
const vec3 blue = vec3(0.0_r, 0.0_r, 1.0_r);
const vec3 black = vec3(0.0_r, 0.0_r, 0.0_r);
const vec3 white = vec3(1.0_r, 1.0_r, 1.0_r);


// share pointers
template<typename T>
using SP = std::shared_ptr<T>;

template<typename T, typename...Params>
SP<T> MakeSP(Params&&...params)
{
    return std::make_shared<T>(std::forward<Params>(params)...);
}

// unique pointers
template<typename T>
using UP = std::unique_ptr<T>;

template<typename T, typename...Params>
UP<T> MakeUP(Params&&...params)
{
    return std::make_unique<T>(std::forward<Params>(params)...);
}

FM_ENGINE_END

#endif