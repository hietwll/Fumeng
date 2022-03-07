#ifndef FUMENG_TEXTURE_H
#define FUMENG_TEXTURE_H

#include <engine/common.h>
#include <engine/core/utils.h>
#include <engine/core/config.h>

FM_ENGINE_BEGIN

class TextureConfig : public Config {
public:
    std::string wrap_u = "repeat";
    std::string wrap_v = "repeat";

    void Load(const nlohmann::json &j) override
    {
        Config::Load(j);
        FM_LOAD_IMPL(j, wrap_u);
        FM_LOAD_IMPL(j, wrap_v);
    }
};

class ConstantTextureConfig : public TextureConfig {
public:
    vec3 value = black;

    void Load(const nlohmann::json &j) override
    {
        TextureConfig::Load(j);
        FM_LOAD_IMPL(j, value);
    }
};

class ImageTextureConfig : public TextureConfig {
public:
    std::string path;
    std::string sampler = "linear";

    ImageTextureConfig() = default;

    ImageTextureConfig(const std::string& image_path) :
    path(image_path)
    {
    }

    void Load(const nlohmann::json &j) override
    {
        TextureConfig::Load(j);
        FM_LOAD_IMPL(j, path);
        FM_LOAD_IMPL(j, sampler);
    }
};

class Texture
{
private:
    using WrapFunc = real(*)(real);
    WrapFunc m_wrap_u = &WrapClamp;
    WrapFunc m_wrap_v = &WrapClamp;

    static void InitWrap(const std::string& wrap_name, WrapFunc& wrap_func)
    {
        if (wrap_name == "clamp") {
            wrap_func = &WrapClamp;
        } else if (wrap_name == "repeat") {
            wrap_func = &WrapRepeat;
        } else {
            spdlog::error("invalid wrap type");
        }
    }

    static real WrapClamp(real x)
    {
        return Clamp(x, 0.0_r, 1.0_r);
    }

    static real WrapRepeat(real x)
    {
        return Clamp(x - std::floor(x), 0.0_r, 1.0_r);
    }

    virtual vec3 SampleImpl(const vec2& uv) const = 0;

public:
    Texture(const TextureConfig& config)
    {
        InitWrap(config.wrap_u, m_wrap_u);
        InitWrap(config.wrap_v, m_wrap_v);
    }

    virtual ~Texture() = default;

    virtual vec3 Sample(const vec2& uv) const
    {
        const real u = m_wrap_u(uv.x);
        const real v = m_wrap_v(uv.y);
        vec3 res = SampleImpl({u, v});
        return res;
    }

    virtual size_t width() const = 0;

    virtual size_t height() const = 0;
};

FM_ENGINE_END

#endif
