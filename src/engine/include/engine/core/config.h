#ifndef FUMENG_ENGINE_CONFIG_H
#define FUMENG_ENGINE_CONFIG_H

#include <engine/common.h>
#include <nlohmann/json.hpp>

FM_ENGINE_BEGIN

#define FM_LOAD_IMPL(x, y) json::LoadValue(x, #y, y)

namespace json {
    template <class T>
    void LoadValue(const nlohmann::json &j, const std::string& name, T& value) {
        if (j.find(name) == j.end()) {
            if constexpr (std::is_same<T, vec3>::value) {
                WarningVec3(name, static_cast<vec3>(value));
            } else {
                WarningDefault(name, value);
            }
            return;
        }

        if constexpr (std::is_same<T, vec3>::value) {
            ParseVec3(j, name, value);
        } else {
            ParseDefault(j, name, value);
        }
    }

    template <class T>
    void ParseDefault(const nlohmann::json &j, const std::string& name, T& value)
    {
        value = j.at(name).get<T>();
    }

    void ParseVec3(const nlohmann::json &j, const std::string& name, vec3& value)
    {
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
    }

    template <class T>
    void WarningDefault(const std::string& name, const T& value)
    {
        spdlog::warn("Value for key {} is not specified, use default: {}", name, value);
    }

    void WarningVec3(const std::string& name, const vec3& value)
    {
        spdlog::warn("Value for {} is not specified, use default: ({}, {}, {})", name,
                     value.x, value.y, value.z);
    }
}

class Config {
public:
    virtual void Load(const nlohmann::json &j) {};
};

FM_ENGINE_END

#endif
