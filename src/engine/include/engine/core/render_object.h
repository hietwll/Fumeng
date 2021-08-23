#ifndef FM_ENGINE_RENDER_OBJECT_H
#define FM_ENGINE_RENDER_OBJECT_H

#include <engine/common.h>
#include <engine/core/geometry.h>
#include <engine/core/material.h>
#include <engine/core/light.h>

FM_ENGINE_BEGIN

class RenderObject
{
private:
    SP<const Geometry> geometry;
    SP<const Material> material;
    SP<const AreaLight> area_light;

public:
    RenderObject(SP<const Geometry>& geometry_, SP<const Material>& material_, vec3 emittance = {0.0_r, 0.0_r, 0.0_r})
    {
        geometry = geometry_;
        material = material_;
        if(!IsBlack(emittance)) {
            area_light = MakeSP<AreaLight>(geometry.get(), emittance);
        }
    }

    bool IsIntersect(const Ray &r) const
    {
        return geometry->IsIntersect(r);
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const
    {
        if(!geometry->GetIntersect(r, hit_point)) {
            return false;
        }

        hit_point->object = this;
        hit_point->material = material.get();

        return true;
    }
};

FM_ENGINE_END