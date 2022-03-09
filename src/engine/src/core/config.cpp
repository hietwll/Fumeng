#include <engine/core/config.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

void TextureDesc::Load(const nlohmann::json &j)
{
    std::string texture_type = "constant";
    json::LoadValue(j, "type", texture_type);

    if (texture_type == "constant") {
        type = TextureType::CONSTANT;
        config = MakeUP<ConstantTextureConfig>();
    } else if (texture_type == "image") {
        type = TextureType::IMAGE;
        config = MakeUP<ImageTextureConfig>();
    } else {
        spdlog::error("texture type not support.");
        throw std::runtime_error("texture type not support.");
    }

    config->Load(j);
}

void TextureDesc::CreateDefault()
{
    config = MakeUP<ConstantTextureConfig>();
}

FM_ENGINE_END