#include <engine/core/geometry.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>
#include "triangle.h"

FM_ENGINE_BEGIN

class Rectangle : public Geometry
{
private:
    // right: m_pa, m_pb, m_pc
    // left: m_pc, m_pd, m_pa
    vec3 m_pa, m_pb, m_pc, m_pd;
    UP<Triangle> m_tri_a, m_tri_b;
    real m_area;
    vec3 m_normal;

public:
    Rectangle(const RectangleConfig& config) :
    m_pa(config.pa),
    m_pb(config.pb),
    m_pc(config.pc),
    m_pd(config.pd),
    Geometry(Transform())
    {
        const auto e0 = m_pb - m_pa;
        const auto e1 = m_pc - m_pa;
        m_normal = glm::cross(e0, e1);
        m_area = glm::length(m_normal);
        m_normal = glm::normalize(m_normal);
    }

    bool IsIntersect(const Ray &r) const override
    {
        vec3 bc;
        real t;
        return triangle_func::IntersectTriangle(r, m_pa, m_pb, m_pc, bc, t) ||
               triangle_func::IntersectTriangle(r, m_pc, m_pd, m_pa, bc, t);
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        vec3 bc;
        real t;

        if (triangle_func::IntersectTriangle(r, m_pa, m_pb, m_pc, bc, t)) {
            CalHitPoint(r, bc, t, hit_point, true);
            return true;
        } else if (triangle_func::IntersectTriangle(r, m_pc, m_pd, m_pa, bc, t)) {
            CalHitPoint(r, bc, t, hit_point, false);
            return true;
        }

        return false;
    }

    void CalHitPoint(const Ray &r, const vec3& bc, real t, HitPoint *hit_point, bool right) const
    {
        hit_point->t = t;
        hit_point->pos = r(t);
        hit_point->wo_r_w = -r.dir;
        hit_point->ng = m_normal;
        hit_point->ns = m_normal;

        vec3 aux = vec3(0.0_r, 0.0_r, 1.0_r);

        if(std::abs(m_normal.z - 1.0_r) < eps ||
        std::abs(m_normal.z + 1.0_r) < eps)
        {
            aux = vec3(1.0_r, 0.0_r, 0.0_r);
        }

        hit_point->ss = glm::cross(m_normal, aux);

        if (right) {
            hit_point->uv = bc.x * vec2(0.0_r, 0.0_r) + 
                            bc.y * vec2(0.0_r, 1.0_r) +
                            bc.z * vec2(1.0_r, 1.0_r);
        } else {
            hit_point->uv = bc.x * vec2(1.0_r, 1.0_r) + 
                            bc.y * vec2(0.0_r, 1.0_r) +
                            bc.z * vec2(0.0_r, 0.0_r);  
        }
    }

    HitPoint Sample(real *pdf, const vec3 &sample) const override
    {
        // get uniformed distributed uv
        const real sqrt_u = std::sqrt(sample.x);
        const real b0 = 1.0_r - sqrt_u;
        const real b1 = sample.y * sqrt_u;
        const real b2 = 1.0_r - b0 - b1;

        HitPoint hitPoint;

        hitPoint.ng = m_normal;
        hitPoint.ns = m_normal;

        vec3 aux = vec3(0.0_r, 0.0_r, 1.0_r);

        if(std::abs(m_normal.z - 1.0_r) < eps ||
        std::abs(m_normal.z + 1.0_r) < eps)
        {
            aux = vec3(1.0_r, 0.0_r, 0.0_r);
        }

        hitPoint.ss = glm::cross(m_normal, aux);        

        if (sample.z < 0.5_r) { // sample right triangle
            hitPoint.pos = b0 * m_pa + b1 * m_pb + b2 * m_pc;
            hitPoint.uv = b0 * vec2(0.0_r, 0.0_r) + 
                           b1 * vec2(0.0_r, 1.0_r) +
                           b2 * vec2(1.0_r, 1.0_r);            
        } else { // sample left triangle
            hitPoint.pos = b0 * m_pc + b1 * m_pd + b2 * m_pa;
            hitPoint.uv = b0 * vec2(1.0_r, 1.0_r) + 
                           b1 * vec2(0.0_r, 1.0_r) +
                           b2 * vec2(0.0_r, 0.0_r);            
        }

        *pdf = Pdf(hitPoint.pos, sample);

        return hitPoint;
    }

    HitPoint Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const override
    {
        return Sample(pdf, sample);
    }

    real Pdf(const vec3& shd_pos, const vec3& sample) const override
    {
        return 1.0 / m_area;
    }

    real Area() const override
    {
        return m_area;
    }

    BBox WorldBound() const override
    {
        BBox box;

        box |= m_pa;
        box |= m_pb;
        box |= m_pc;
        box |= m_pd;
        
        return box;
    }
};

FM_ENGINE_END