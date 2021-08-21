#ifndef FM_ENGINE_GEOMETRY_H
#define FM_ENGINE_GEOMETRY_H

#include <engine/common.h>
#include <engine/core/transform.h>

FM_ENGINE_BEGIN

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
     */
    virtual bool GetIntersect(const Ray &r, HitPoint *hit_point) const = 0;

    /**
     * Get the area of the shape
     */
    virtual real Area() const = 0;

    /**
     * Sample a point on the surface and return pdf (to area).
     */
    virtual HitPoint Sample(real *pdf, const vec3 &sample) const = 0;

    /**
     * Get the pdf of a point.
     */
    virtual real Pdf(const vec3 &pos) const
    {
        return 1.0_r / Area();
    };
};

FM_ENGINE_END

#endif