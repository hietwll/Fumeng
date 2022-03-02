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
#include <fstream>
#include <optional>

FM_ENGINE_BEGIN

class SceneLoader {
private:
    nlohmann::json scene_config;
    SP<Camera> m_camera;
    SP<Renderer> m_render;

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
        CreateRender();
    }

    void CreateCamera()
    {
        auto c = GetOptional(scene_config, "camera");
        if (c) {
            std::string camera_type = "pin_hole";
            json::LoadValue(c->get(), "type", camera_type);

            if (camera_type == "pin_hole") {
//                m_camera = CreatePinPoleCamera(
//                        GetVec3(c->get(), "pos", black),
//                        GetVec3(c->get(), "look_at", red),
//                        GetVec3(c->get(), "up", blue),
//                        GetDeFault(c->get(), "focal_distance", 1.0_r),
//                        GetDeFault(c->get(), "fov", 60.0_r),
//                        GetDeFault(c->get(), "aspect", 1.0_r));
                return;
            }
        }

        throw std::runtime_error("Camera is not specified.");
    }

    void CreateRender()
    {
        auto c = GetOptional(scene_config, "renderer");
        if (c) {
            std::string render_type = "path_tracer";
            json::LoadValue(c->get(), "type", render_type);
            if (render_type == "path_tracer") {
                PathTracerConfig config;
                config.Load(c->get());
                return;
            }
        }

        throw std::runtime_error("Camera is not specified.");
    }

    void CreateScene()
    {

    }
};

FM_ENGINE_END

#endif
