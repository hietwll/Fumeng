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
    real radius_world;
    real rw2; // radius_world * radius_world
    real area;

public:
    Sphere(real r, const Transform& to_world) :
    radius(r), Geometry(to_world)
    {
        radius_world = radius *
                glm::length(to_world.ApplyToVec3(vec3(1.0_r, 0.0_r, 0.0_r)));
        rw2 = radius_world * radius_world;
        area = 4.0_r * PI * rw2;
    }

    Sphere(const SphereConfig& config) :
    Sphere(config.radius, Transform(config.translation, config.rotation, config.scale))
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

        CalHitPoint(pos, hit_point);
        hit_point->t = t;
        hit_point->wo_r_w = -r.dir;

        return true;
    }

    /*
     * Fill HitPoint(in world space) based on given hit position.
     */
    void CalHitPoint(const vec3& pos, HitPoint* hit_point) const
    {
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

    HitPoint Sample(real *pdf, const vec3 &sample) const override
    {
        vec2 samp = vec2(sample.x, sample.y);
        vec3 pos = UniformSphere(samp) * radius;

        *pdf = 1.0_r / area;

        HitPoint hit_point;
        CalHitPoint(pos, &hit_point);
        return hit_point;
    }

    HitPoint Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const override
    {
        // shading point in local space
        const vec3 sp_l = world_to_object.ApplyToPoint(ref.pos);
        const real dis = glm::length(sp_l);

        // if the shading point is inside sphere, then sampling the whole sphere
        if(dis < radius) {
            return Sample(pdf, sample);
        }

        // else sample on cone, visibility is tested in MIS.
        const real cos_theta = std::min(radius / dis, 1.0_r);
        vec3 dir_samp;
        real pdf_area;
        const vec2 samp = vec2(sample.x, sample.y);

        UniformCone(cos_theta, samp, &dir_samp, &pdf_area);

        // transform a locally-sampled pos to sphere space
        const vec3 axis_z = glm::normalize(sp_l);
        vec3 axis_x, axis_y;
        CreateCoordSys(axis_z, &axis_x, &axis_y);

        const vec3 pos = radius *
                (axis_x * dir_samp.x + axis_y * dir_samp.y + axis_z * dir_samp.z);

        HitPoint hit_point;
        CalHitPoint(pos, &hit_point);

        // pdf_area is based on unit sphere
        *pdf = pdf_area / rw2;
        return hit_point;
    }

    real Pdf(const vec3& shd_pos, const vec3& sample) const override
    {
        // shading point in local space
        const vec3 sp_l = world_to_object.ApplyToPoint(shd_pos);
        const real dis = glm::length(sp_l);

        // if the shading point is inside sphere, then sampling the whole sphere
        if(dis < radius) {
            return Geometry::Pdf(sample);
        }

        // else sample on cone
        const real cos_theta = std::min(radius / dis, 1.0_r);

        return 1.0_r / (2.0_r * PI * (1.0_r - cos_theta)) / rw2;
    }

    /*
     * Get the area
     */
    real Area() const override
    {
        return area;
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

    BBox WorldBound() const override
    {
        const vec3 center_world = object_to_world.ApplyToPoint(black);
        return {center_world - vec3(radius_world), center_world + vec3(radius_world)};
    }
};

SP<Geometry> CreateSphere(real radius, const Transform& to_world)
{
    return MakeSP<Sphere>(radius, to_world);
}

SP<Geometry> CreateSphere(const SphereConfig& config)
{
    return MakeSP<Sphere>(config);
}

FM_ENGINE_END