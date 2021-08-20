#ifndef FM_ENGINE_COMMON_H
#define FM_ENGINE_COMMON_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>

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

const real PI = 3.14159265354_r;
const real InvPI = 1.0_r / PI;

// forward declaration
class Ray;
class Scene;
class Sampler;
class Camera;
class Transform;
class HitPoint;
class BSDF;

// share pointers
template<typename T>
using SP = std::shared_ptr<T>;

template<typename T, typename...Params>
SP<T> makeSP(Params&&...params)
{
    return std::make_shared<T>(std::forward<Params>(params)...);
}

// unique pointers
template<typename T>
using UP = std::unique_ptr<T>;

template<typename T, typename...Params>
UP<T> makeUP(Params&&...params)
{
    return std::make_unique<T>(std::forward<Params>(params)...);
}

FM_ENGINE_END

#endif