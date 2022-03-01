#ifndef FUMENG_ENGINE_SCENE_LOADER_H
#define FUMENG_ENGINE_SCENE_LOADER_H

#include <engine/create/geometry.h>
#include <engine/create/camera.h>
#include <engine/create/material.h>
#include <engine/create/render_object.h>
#include <engine/create/aggregate.h>
#include <engine/create/renderer.h>
#include <engine/create/scene.h>
#include <engine/create/texture.h>
#include <engine/create/light.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>

FM_ENGINE_BEGIN

class SceneLoader {
private:
    nlohmann::json scene_config;
    SP<Camera> m_camera;

    template <class T>
    inline T GetDeFault(const nlohmann::json &j, const std::string& field, const T& default_value) const
    {
        T ret = default_value;
        if (j.find(field) == j.end()) {
            spdlog::warn("Value for key {} is not specified, use default: {}.", field, default_value);
            return ret;
        }
        ret = j.at(field).get<T>();
        return ret;
    }

    vec3 GetVec3(const nlohmann::json &j, const std::string& field, const vec3& default_value) const
    {
        if (j.find(field) == j.end()) {
            spdlog::warn("Value for key {} is not specified, use default: {},{}, {}.", field,
                         default_value.x, default_value.y, default_value.z);
            return default_value;
        }

        auto number = j.at(field).get<std::vector<real>>();

        if (number.size() == 0) {
            return default_value;
        } else if (number.size() < 3) {
            return vec3{number[0]};
        } else {
            return vec3{number[0], number[1], number[2]};
        }
    }

    std::optional<std::reference_wrapper<const nlohmann::json>> GetOptional(const nlohmann::json &j, const std::string& field) const
    {
        if (j.find(field) != j.end()) {
            return j.at(field);
        }
        return std::nullopt;
    }

public:
    SceneLoader() {};

    void Load(const std::string& path)
    {
        std::ifstream ifs(path);
        scene_config = nlohmann::json::parse(ifs);
        CreateCamera();
    }

    void CreateCamera()
    {
        auto c = GetOptional(scene_config, "camera");
        if (c) {
            std::string default_type = "pin_hole";
            std::string camera_type = GetDeFault(c->get(), "type", default_type);
            if (camera_type == "pin_hole") {
                m_camera = CreatePinPoleCamera(
                        GetVec3(c->get(), "pos", black),
                        GetVec3(c->get(), "look_at", red),
                        GetVec3(c->get(), "up", blue),
                        GetDeFault(c->get(), "focal_distance", 1.0_r),
                        GetDeFault(c->get(), "fov", 60.0_r),
                        GetDeFault(c->get(), "aspect", 1.0_r));
            }
        }
    }

    void CreateRender()
    {

    }

    void CreateScene()
    {

    }
};

FM_ENGINE_END

#endif
