#ifndef FUMENG_ENGINE_CONFIG_H
#define FUMENG_ENGINE_CONFIG_H

#include <engine/common.h>
#include <nlohmann/json.hpp>


FM_ENGINE_BEGIN

#define FM_LOAD_IMPL(x, y) json::LoadValue(x, #y, y)

class Config {
public:
    virtual void Load(const nlohmann::json &j) {};
};

class TextureDesc : public Config {
public:
    enum class TextureType
    {
        CONSTANT,
        IMAGE
    };

    UP<Config> config;
    TextureType type {TextureType::CONSTANT};
    void Load(const nlohmann::json &j) override;
    void CreateDefault();
};

namespace json {
    template <class T>
    static void LoadValue(const nlohmann::json &j, const std::string& name, T& value);

    template <class T>
    static void ParseDefault(const nlohmann::json &j, const std::string& name, T& value);

    static void ParseVec3(const nlohmann::json &j, const std::string& name, vec3& value);

    static void ParseTextureDesc(const nlohmann::json &j, const std::string& name, TextureDesc& value);

    template <class T>
    static void WarningDefault(const std::string& name, const T& value);

    static void WarningVec3(const std::string& name, const vec3& value);
}

FM_ENGINE_END

#endif
