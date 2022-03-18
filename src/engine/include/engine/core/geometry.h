#ifndef FM_ENGINE_GEOMETRY_H
#define FM_ENGINE_GEOMETRY_H

#include <engine/common.h>
#include <engine/core/transform.h>
#include <engine/core/bbox.h>
#include <engine/core/config.h>

FM_ENGINE_BEGIN

class RectangleConfig : public Config
{
public:
    vec3 pa, pb, pc, pd;

    void Load(const nlohmann::json &j) override
    {
        FM_LOAD_IMPL(j, pa);
        FM_LOAD_IMPL(j, pb);
        FM_LOAD_IMPL(j, pc);
        FM_LOAD_IMPL(j, pd);
    }    
};

class SphereConfig : public Config
{
public:
    real radius {1.0_r};
    vec3 translation {black};
    vec3 rotation {black};
    vec3 scale {white};

    void Load(const nlohmann::json &j) override
    {
        FM_LOAD_IMPL(j, radius);
        FM_LOAD_IMPL(j, translation);
        FM_LOAD_IMPL(j, rotation);
        FM_LOAD_IMPL(j, scale);
    }    
};

class Geometry
{
protected:
    Transform object_to_world;
    Transform world_to_object;
public:
    Geometry(const Transform& to_world) :
    object_to_world(to_world), world_to_object(object_to_world.InvTransform())
    {
    }

    virtual ~Geometry() = default;

    /**
     * Test if the ray intersect with this geometry
     * Ray is in world space
     */
    virtual bool IsIntersect(const Ray &r) const = 0;

    /**
     * Get the HitPoint information if there is a intersection
     * Both Ray and HitPoint should be in world space
     * Ray is in world space
     */
    virtual bool GetIntersect(const Ray &r, HitPoint *hit_point) const = 0;

    /**
     * Get the area of the shape
     */
    virtual real Area() const = 0;

    /**
     * Sample a point on the surface and return pdf (to area).
     * By default, we use a uniform distribution over surface area.
     */
    virtual HitPoint Sample(real *pdf, const vec3 &sample) const = 0;

    /**
     * Get the pdf of a point, w.r.t. area
     */
    virtual real Pdf(const vec3& shd_pos) const
    {
        return 1.0_r / Area();
    };

    /**
     * For sampling light, a more efficient way is to only sample point that is visible
     * to the point to be shaded (i.e. ref).
     * It doesn't make sense to calculate the pdf w.r.t solid angle here, because that
     * breaks the uniformity. Here we still return the pdf w.r.t surface area and get
     * the pdf w.r.t solid angle in MIS.
     */
    virtual HitPoint Sample(const HitPoint& ref, real *pdf, const vec3& sample) const = 0;

    /*
     * Get the pdf of w.r.t. area by sampling the point visible by ref
     */
    virtual real Pdf(const vec3& shd_pos, const vec3& sample) const = 0;

    /**
     * Get bounding box in world space
     */
     virtual BBox WorldBound() const = 0;
};

FM_ENGINE_END

#endif