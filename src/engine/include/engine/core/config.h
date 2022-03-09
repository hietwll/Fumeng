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
    static void LoadValue(const nlohmann::json &j, const std::string& name, T& value) {
        if (j.find(name) == j.end()) {
            if constexpr (std::is_same<T, vec3>::value) {
                WarningVec3(name, static_cast<vec3>(value));
            } else if constexpr(std::is_same<T, TextureDesc>::value) {
                WarningTextureDesc(name, value);
            } else {
                WarningDefault(name, value);
            }
            return;
        }

        if constexpr (std::is_same<T, vec3>::value) {
            ParseVec3(j, name, value);
        } else if constexpr(std::is_same<T, TextureDesc>::value) {
            ParseTextureDesc(j, name, value);
        } else {
            ParseDefault(j, name, value);
        }
    }

    template <class T>
    static void ParseDefault(const nlohmann::json &j, const std::string& name, T& value)
    {
        try {
            value = j.at(name).get<T>();
        } catch (...) {
            spdlog::error("Failed when parsing {}.", name);
        }
    }

    static void ParseVec3(const nlohmann::json &j, const std::string& name, vec3& value)
    {
        try {
            auto number = j.at(name).get<std::vector<real>>();
            if (number.size() == 0) {
                return;
            } else if (number.size() < 3) {
                value.x = value.y = value.z = number[0];
                return;
            } else {
                value.x = number[0];
                value.y = number[1];
                value.z = number[2];
                return;
            }
        } catch (...) {
            spdlog::error("Failed when parsing {}.", name);
        }
    }

    static void ParseTextureDesc(const nlohmann::json &j, const std::string& name, TextureDesc& value)
    {
        value.Load(j.at(name));
    }

    template <class T>
    static void WarningDefault(const std::string& name, const T& value)
    {
        spdlog::warn("Value for key {} is not specified, use default: {}", name, value);
    }

    static void WarningVec3(const std::string& name, const vec3& value)
    {
        spdlog::warn("Value for {} is not specified, use default: ({}, {}, {})", name,
                     value.x, value.y, value.z);
    }

    static void WarningTextureDesc(const std::string& name, TextureDesc& value)
    {
        value.CreateDefault();
        spdlog::warn("Value for {} is not specified, use default: (0, 0, 0)", name);
    }
}

FM_ENGINE_END

#endif
