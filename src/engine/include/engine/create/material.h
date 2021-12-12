#ifndef FM_ENGINE_CREATE_MATERIAL_H
#define FM_ENGINE_CREATE_MATERIAL_H

#include <engine/core/material.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

SP<Material> CreateLambertDiffuse(const SP<Texture>& color);

SP<Material> CreateSpecularReflection(const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k);

SP<Material> CreateSpecularTransmission(const vec3& reflect_color_, const vec3& refract_color_, const real eta_i, const real eta_t);

SP<Material> CreateDisneyMaterial(
        SP<Texture> basecolor,
        SP<Texture> metallic,
        SP<Texture> specular,
        SP<Texture> specularTint,
        SP<Texture> roughness,
        SP<Texture> anisotropic,
        SP<Texture> sheen,
        SP<Texture> sheenTint,
        SP<Texture> clearcoat,
        SP<Texture> clearcoatGloss,
        SP<Texture> specTrans,
        SP<Texture> diffTrans,
        SP<Texture> flatness,
        SP<Texture> ior,
        const bool thin);

FM_ENGINE_END

#endif