#ifndef FM_ENGINE_RENDER_OBJECT_H
#define FM_ENGINE_RENDER_OBJECT_H

#include <engine/common.h>
#include <engine/core/geometry.h>
#include <engine/core/material.h>
#include <engine/core/light.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>
#include <engine/core/bbox.h>

#include <vector>

FM_ENGINE_BEGIN

class RenderObject
{
private:
    SP<const Geometry> m_geometry;
    SP<const Material> m_material;
    SP<const AreaLight> m_area_light;
    bool m_emissive = false;

public:
    RenderObject(SP<const Geometry>& geometry, SP<const Material>& material, vec3 emittance = {0.0_r, 0.0_r, 0.0_r})
    {
        m_geometry = geometry;
        m_material = material;
        if(!IsBlack(emittance)) {
            m_area_light = MakeSP<AreaLight>(m_geometry.get(), emittance);
            m_emissive = true;
        }
    }

    bool IsIntersect(const Ray &r) const
    {
        return m_geometry->IsIntersect(r);
    }

    bool IsEmissive() const
    {
        return m_emissive;
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const
    {
        if(!m_geometry->GetIntersect(r, hit_point)) {
            return false;
        }

        hit_point->object = this;
        hit_point->material = m_material.get();

        return true;
    }

    void ConstructAreaLight(std::vector<SP<const Light>>& lights) const
    {
        if(m_emissive) {
            lights.push_back(m_area_light);
        }
    }

    const AreaLight* GetLight() const
    {
        return m_area_light.get();
    }

    BBox WorldBound() const noexcept
    {
        return m_geometry->WorldBound();
    }

    size_t PrimCount() const
    {
        return m_geometry->PrimCount();
    }
};

FM_ENGINE_END

#endif