#ifndef FM_ENGINE_CREATE_MATERIAL_H
#define FM_ENGINE_CREATE_MATERIAL_H

#include <engine/core/material.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

SP<Material> CreateLambertDiffuse(const LambertDiffuseConfig& config);

SP<Material> CreateSpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k);

SP<Material> CreateSpecularTransmission(const SpecularTransmissionConfig& config);

SP<Material> CreateDisneyMaterial(const DisneyConfig& config);

FM_ENGINE_END

#endif