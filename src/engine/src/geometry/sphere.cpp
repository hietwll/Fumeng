#include <engine/core/geometry.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>

#include <tuple>

FM_ENGINE_BEGIN



class Sphere : public Geometry
{
private:
    real radius;

public:
    Sphere(real r, const Transform& to_world)
    : radius(r), Geometry(to_world)
    {

    }

    bool IsIntersect(const Ray &r) const override
    {
        Ray r_l;
        real t;
        return IntersectSphere(r, r_l, t);
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        Ray r_l;
        real t;

        if(!IntersectSphere(r, r_l, t)) {
            return false;
        }

        vec3 pos = r_l(t);

        // Get UV coordinate;
        real phi = (pos.x == 0 && !pos.y) ? 0.0_r : std::atan2(pos.y, pos.x);
        if(phi < 0) {
            phi += 2.0_r * PI;
        }

        const real cos_theta = Clamp(pos.z / radius, -1.0_r, 1.0_r);
        const real theta = std::acos(cos_theta);

        vec3 geom_norm = glm::normalize(pos);
        vec3 aux = vec3(0.0_r, 0.0_r, 1.0_r);

        if(std::abs(geom_norm.z - 1.0_r) < eps ||
            std::abs(geom_norm.z + 1.0_r) < eps)
        {
            aux = vec3(1.0_r, 0.0_r, 0.0_r);
        }

        hit_point->uv.x = phi * 0.5_r * InvPI;
        hit_point->uv.y = theta * InvPI;
        hit_point->pos = pos;
        hit_point->ng = geom_norm;
        hit_point->ns = geom_norm;
        hit_point->ss = glm::cross(geom_norm, aux);

        object_to_world.ApplyToHitPoint(hit_point);

        return true;
    }

    bool IntersectSphere(const Ray &r, Ray& r_l, real& t) const
    {
        r_l = world_to_object.ApplyToRay(r);
        const auto [a, b, c] = GetCoef(r_l);
        real t0, t1;

        if(!SolveQuadratic(a, b, c, t0, t1)) {
            return false;
        }

        if(r_l.Contains(t0)) {
            t = t0;
            return true;
        }

        if(r_l.Contains(t1))
        {
            t = t1;
            return true;
        }

        return false;
    }

    bool SolveQuadratic(const real& a, const real& b, const real& c, real& t0, real& t1) const
    {
        real delta = b * b - 4.0_r * a * c;
        if (delta < 0) {
            return false;
        }

        delta = std::sqrt(delta);

        const real inv2a = 0.5_r / a;
        t0 = (-b - delta) * inv2a;
        t1 = (-b + delta) * inv2a;

        return true;
    }

    /*
     * Get coefficient for equation: at^2 + bt + c = 0
     */
    std::tuple<real, real, real> GetCoef(const Ray& r) const
    {
        const real a = SquareSum(r.dir);
        const real b = 2.0_r * glm::dot(r.dir, r.ori);
        const real c = SquareSum(r.ori) - radius * radius;
        return {a, b, c};
    }
};


FM_ENGINE_END