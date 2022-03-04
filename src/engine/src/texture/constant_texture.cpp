#include <engine/core/texture.h>

FM_ENGINE_BEGIN

class ConstantTexture : public Texture
{
private:
    vec3 m_value;

    vec3 SampleImpl(const vec2& uv) const override
    {
        return m_value;
    }

public:
    ConstantTexture(const ConstantTextureConfig& config) :
    Texture(config),
    m_value(config.value)
    {
    }

    size_t width() const override
    {
        return 1;
    }

    size_t height() const override
    {
        return 1;
    }
};

SP<Texture> CreateConstantTexture(const ConstantTextureConfig& config)
{
    return MakeSP<ConstantTexture>(config);
}

FM_ENGINE_END
