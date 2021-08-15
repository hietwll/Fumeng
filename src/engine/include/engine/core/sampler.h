#ifndef FM_ENGINE_RAY_H
#define FM_ENGINE_RAY_H

#include <engine/common.h>

#include <chrono>
#include <random>

FM_ENGINE_BEGIN

class Sampler
{
public:
    using rng_t = std::mt19937_64;
    using udis_t = std::uniform_real_distribution<real>;
    rng_t rng;
    udis_t udis;
    size_t seed;

    Sampler()
    {
        seed = static_cast<size_t>
            (std::chrono::high_resolution_clock::now().time_since_epoch().count());
        rng = rng_t(seed);
    };

    real Get1D() const
    {
        return udis(rng);
    }

    vec2 Get2D() const
    {
        return {Get1D(), Get1D()};
    }

};

FM_ENGINE_END

#endif