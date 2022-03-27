#include <engine/core/light.h>

FM_ENGINE_BEGIN

AreaLight::AreaLight(const Geometry *geometry, const vec3 &radiance)
: m_geometry(geometry), m_radiance(radiance)
{
}

LightSampleInfo AreaLight::Sample(const HitPoint &hit_point, const vec3 &sample) const
{
    real pdf_area; // returned pdf is w.r.t. geometry area
    auto geom_sample = m_geometry->Sample(hit_point, &pdf_area, sample);

    const vec3 ref_to_light = geom_sample.pos - hit_point.pos;
    const real dist = glm::length(ref_to_light);

    if (dist < eps) {
        return {black, black, black, black, 0.0_r, 0.0_r};
    }

    const vec3 wi = glm::normalize(ref_to_light);
    const real pdf_solid_angle = pdf_area * dist * dist / AbsDot(geom_sample.ng, -wi);

    return {hit_point.pos, geom_sample.pos, wi, m_radiance, pdf_solid_angle, dist};
}

bool AreaLight::IsDelta() const
{
    return false;
}

vec3 AreaLight::GetRadiance(const vec3& pos, const vec3& nor, const vec2& uv, const vec3& light_to_shd) const
{
    return glm::dot(nor, light_to_shd) > 0 ? m_radiance : black;
}

real AreaLight::Pdf(const vec3& shd_pos, const vec3& light_pos, const vec3& light_nor, const vec3& light_to_shd) const
{
    if (glm::dot(light_nor, light_to_shd) <0) {
        return 0.0_r;
    }

    // get pdf w.r.t. area from geometry
    const real pdf_area = m_geometry->Pdf(shd_pos, light_pos);

    // get pdf w.r.t. solid angle
    const real dist = glm::length(shd_pos - light_pos);
    return pdf_area * dist * dist / AbsDot(light_nor, light_to_shd);
}

FM_ENGINE_END
