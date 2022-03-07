#include <engine/core/texture.h>

FM_ENGINE_BEGIN

SP<Texture> CreateImageTexture(const ImageTextureConfig& config);

SP<Texture> CreateConstantTexture(const ConstantTextureConfig& config);

SP<Texture> CreateTexture(const TextureDesc& desc)
{
    if (desc.type == TextureDesc::TextureType::CONSTANT) {
        return CreateConstantTexture(*dynamic_cast<ConstantTextureConfig*>(desc.config.get()));
    } else if (desc.type == TextureDesc::TextureType::IMAGE) {
        return CreateImageTexture(*dynamic_cast<ImageTextureConfig*>(desc.config.get()));
    } else {
        throw std::runtime_error("Texture type not supported.");
    }
}

FM_ENGINE_END;
