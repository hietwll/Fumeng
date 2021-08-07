#ifndef FM_ENGINE_COMMON_H
#define FM_ENGINE_COMMON_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using real = float;
using dreal = double;

constexpr real REAL_MAX = std::numeric_limits<real>::max();

#define FM_ENGINE_BEGIN namespace fumeng::engine {
#define FM_ENGINE_END }

#endif