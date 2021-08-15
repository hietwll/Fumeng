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

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using real = float;
using dreal = double;

constexpr real REAL_MAX = std::numeric_limits<real>::max();

// forward declaration
class Ray;
class Scene;
class Sampler;
class Camera;
class Transform;
class HitPoint;

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