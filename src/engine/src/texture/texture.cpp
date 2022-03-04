#include <engine/core/texture.h>

FM_ENGINE_BEGIN

SP<Texture> CreateImageTexture(const ImageTextureConfig& config);

SP<Texture> CreateConstantTexture(const ConstantTextureConfig& config);

SP<Texture> CreateTexture(const TextureDesc& desc)
{
    if (desc.type == TextureDesc::TextureType::CONSTANT) {
        return CreateConstantTexture(desc.config);
    } else if (desc.type == TextureDesc::TextureType::IMAGE) {
        return CreateImageTexture(desc.config);
    } else {
        throw throw std::runtime_error("Texture type not supported.");
    }
}

FM_ENGINE_END;
