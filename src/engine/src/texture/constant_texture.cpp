#include <engine/core/texture.h>
#include <engine/core/image.h>

FM_ENGINE_BEGIN

class ConstantTexture : public Texture
{
private:
    vec3 value;

    vec3 SampleImpl(const vec2& uv) const override
    {
        return value;
    }

public:
    ConstantTexture(const vec3& value_): Texture(), value(value_)
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

SP<Texture> CreateConstantTexture(const vec3& value)
{
    return MakeSP<ConstantTexture>(value);
}

FM_ENGINE_END
