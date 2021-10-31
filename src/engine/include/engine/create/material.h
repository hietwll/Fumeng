#ifndef FM_ENGINE_CREATE_MATERIAL_H
#define FM_ENGINE_CREATE_MATERIAL_H

#include <engine/core/material.h>

FM_ENGINE_BEGIN

SP<Material> CreateLambertDiffuse(const vec3& color);

SP<Material> CreateSpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k);

SP<Material> CreateSpecularTransmission(const vec3& reflect_color_, const vec3& refract_color_, const real eta_i, const real eta_t);

FM_ENGINE_END

#endif