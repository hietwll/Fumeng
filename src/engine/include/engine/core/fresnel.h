#ifndef FM_ENGINE_FRENEL_H
#define FM_ENGINE_FRENEL_H

#include <engine/common.h>

FM_ENGINE_BEGIN

class Fresnel
{
public:
    virtual ~Fresnel() = default;
    virtual vec3 CalFr(real cos_i) const = 0;
};


class DielectricFresnel : public Fresnel
{
private:
    real eta_i, eta_t;

public:
    DielectricFresnel(real eta_i_, real eta_t_)
    : eta_i(eta_i_), eta_t(eta_t_)
    {
    };

    /*
     * Calculate fresnel reflectance for dielectric
     */
    vec3 CalFr(real cos_i) const override
    {
        real eta_in = eta_i; // incident media
        real eta_out = eta_t; // transmitted media
        if(cos_i < 0)
        {
            std::swap(eta_in, eta_out); // incident ray is on the inside
            cos_i = -cos_i; // make sure cos_i is greater than zero.
        }

        const real sin_i = std::sqrt((std::max)(0.0_r, 1.0_r - cos_i * cos_i));
        const real sin_t = eta_in / eta_out * sin_i;

        if(sin_t >= 1.0_r) {
            return {1.0_r, 1.0_r, 1.0_r};
        }

        const real cos_t = std::sqrt((std::max)(0.0_r, 1.0_r - sin_t * sin_t));

        const real perpendicular = (eta_in * cos_i - eta_out * cos_t)
                / (eta_in * cos_i + eta_out * cos_t);
        const real parallel = (eta_out * cos_i - eta_in * cos_t)
                / (eta_out * cos_i + eta_in * cos_t);

        const real fr = (parallel * parallel + perpendicular * perpendicular) * 0.5_r;

        return {fr, fr, fr};
    };

    real EtaI() const
    {
        return eta_i;
    }

    real EtaT() const
    {
        return eta_t;
    }
};

class ConductorFresnel : public Fresnel
{
private:
    vec3 eta2, etak2;

public:
    ConductorFresnel(const vec3& eta_i, const vec3& eta_t, const vec3& k)
    {
        eta2 = eta_t / eta_i;
        eta2 *= eta2;

        etak2 = k  / eta_i;
        etak2 *= etak2;
    };

    /*
     * Calculate fresnel reflectance for conductor
     */
    vec3 CalFr(real cos_i) const override
    {
        if(cos_i <= 0.0_r) {
            return {};
        }

        const real cos2 = cos_i * cos_i;
        const real sin2 = (std::max)(0.0_r, 1.0_r - cos2);

        const vec3 t0 = eta2 - etak2 - sin2;
        const vec3 a2b2 = sqrt((t0 * t0 + 4.0_r * eta2 * etak2));
        const vec3 t1 = a2b2 + cos2;
        const vec3 a = sqrt(0.5_r * (a2b2 + t0));
        const vec3 t2 = 2.0_r * cos_i * a;
        const vec3 rs = (t1 - t2) / (t1 + t2);

        const vec3 t3 = cos2 * a2b2 + sin2 * sin2;
        const vec3 t4 = t2 * sin2;
        const vec3 rp = rs * (t3 - t4) / (t3 + t4);

        return (rp + rs) * 0.5_r;
    };
};

FM_ENGINE_END

#endif