#ifndef FM_ENGINE_MATERIAl_H
#define FM_ENGINE_MATERIAl_H

#include <engine/common.h>
#include <engine/core/bsdf.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

class LambertDiffuseConfig : public Config {
public:
    TextureDesc basecolor {white};

    void Load(const nlohmann::json &j) override
    {
        FM_LOAD_IMPL(j, basecolor);
    }
};

class DisneyConfig : public Config {
public:
    TextureDesc basecolor;
    TextureDesc metallic;
    TextureDesc specular;
    TextureDesc specularTint;
    TextureDesc roughness;
    TextureDesc anisotropic;
    TextureDesc sheen;
    TextureDesc sheenTint;
    TextureDesc clearcoat;
    TextureDesc clearcoatGloss;
    TextureDesc specTrans;
    TextureDesc specTransRoughness;
    TextureDesc diffTrans;
    TextureDesc flatness;
    TextureDesc ior {1.5};
    bool thin = false;

    void Load(const nlohmann::json &j) override
    {
        Config::Load(j);
        FM_LOAD_IMPL(j, basecolor);
        FM_LOAD_IMPL(j, metallic);
        FM_LOAD_IMPL(j, specular);
        FM_LOAD_IMPL(j, specularTint);
        FM_LOAD_IMPL(j, roughness);
        FM_LOAD_IMPL(j, anisotropic);
        FM_LOAD_IMPL(j, sheen);
        FM_LOAD_IMPL(j, sheenTint);
        FM_LOAD_IMPL(j, clearcoat);
        FM_LOAD_IMPL(j, clearcoatGloss);
        FM_LOAD_IMPL(j, specTrans);
        FM_LOAD_IMPL(j, specTransRoughness);
        FM_LOAD_IMPL(j, diffTrans);
        FM_LOAD_IMPL(j, specTrans);
        FM_LOAD_IMPL(j, flatness);
        FM_LOAD_IMPL(j, ior);
        FM_LOAD_IMPL(j, thin);
    }
};

class Material
{
public:
    virtual ~Material() = default;
    virtual void CreateBSDF(HitPoint& hit_point) const = 0;
};

FM_ENGINE_END

#endif