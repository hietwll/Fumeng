#include <engine/core/config.h>
#include <engine/core/texture.h>

FM_ENGINE_BEGIN

//namespace json {
//    template <class T>
//    static void LoadValue(const nlohmann::json &j, const std::string& name, T& value) {
//        if (j.find(name) == j.end()) {
//            if constexpr (std::is_same<T, vec3>::value) {
//                WarningVec3(name, static_cast<vec3>(value));
//            } else if constexpr(std::is_same<T, TextureDesc>::value) {
//                WarningTextureDesc(j, name, value);
//            } else {
//                WarningDefault(name, value);
//            }
//            return;
//        }
//
//        if constexpr (std::is_same<T, vec3>::value) {
//            ParseVec3(j, name, value);
//        } else if constexpr(std::is_same<T, TextureDesc>::value) {
//            ParseTextureDesc(j, name, value);
//        } else {
//            ParseDefault(j, name, value);
//        }
//    }
//
//    template <class T>
//    static void ParseDefault(const nlohmann::json &j, const std::string& name, T& value)
//    {
//        value = j.at(name).get<T>();
//    }
//
//    static void ParseVec3(const nlohmann::json &j, const std::string& name, vec3& value)
//    {
//        auto number = j.at(name).get<std::vector<real>>();
//
//        if (number.size() == 0) {
//            return;
//        } else if (number.size() < 3) {
//            value.x = value.y = value.z = number[0];
//            return;
//        } else {
//            value.x = number[0];
//            value.y = number[1];
//            value.z = number[2];
//            return;
//        }
//    }
//
//    static void ParseTextureDesc(const nlohmann::json &j, const std::string& name, TextureDesc& value)
//    {
//        value.Load(j.at(name));
//    }
//
//    template <class T>
//    static void WarningDefault(const std::string& name, const T& value)
//    {
//        spdlog::warn("Value for key {} is not specified, use default: {}", name, value);
//    }
//
//    static void WarningVec3(const std::string& name, const vec3& value)
//    {
//        spdlog::warn("Value for {} is not specified, use default: ({}, {}, {})", name,
//                     value.x, value.y, value.z);
//    }
//
//    static void WarningTextureDesc(const std::string& name, TextureDesc& value)
//    {
//        value.CreateDefault();
//        spdlog::warn("Value for {} is not specified, use default: (0, 0, 0)");
//    }
//}

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