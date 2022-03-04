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
                PinHoleCameraConfig config;
                config.Load(c->get());
                m_camera = CreatePinPoleCamera(config);
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

        auto c = GetOptional(scene_config, "scene");
        if (c) {
            // create render objects
            auto obj_configs = GetOptional(c->get(), "render_objects");
            std::vector<SP<const RenderObject>> render_objects;
            if (obj_configs) {
                for(auto& config : obj_configs->get()) {
                    CreateRenderObjects(config, render_objects);
                }
            }

            // create accelerator


            // create env
        }

        throw std::runtime_error("Scene is not specified.");
    }

    void CreateRenderObjects(const nlohmann::json &j, std::vector<SP<const RenderObject>>& objs)
    {
        // create shape
        std::vector<SP<const Geometry>> geometries;
        auto shape_config = GetOptional(j, "shape");
        if (shape_config) {
            std::string shape_type;
            json::LoadValue(shape_config->get(), "type", shape_type);
            if (shape_type == "triangle_mesh") {
                std::string path;
                json::LoadValue(shape_config->get(), "path", path);
                CreateTriangleMesh(path, geometries);
            } else if (shape_type == "sphere") {
                // todo: fill spere loading
            } else {
                spdlog::error("Shape type not supported: {}.", shape_type);
                throw std::runtime_error("Shape type not supported.");
            }
        }

        // create material
        auto material_config = GetOptional(j, "material");
        SP<const Material> material;
        if (material_config) {
            std::string material_type;
            json::LoadValue(shape_config->get(), "type", material_type);
            if (material_type == "lambert_diffuse") {
                LambertDiffuseConfig config;
                config.Load(j);
                material = CreateLambertDiffuse(config);
            } else if (material_type == "disney") {
                DisneyConfig config;
                config.Load(j);
                material = CreateDisneyMaterial(config);
            } else {
                spdlog::error("Shape type not supported: {}.", material_type);
                throw std::runtime_error("Shape type not supported.");
            }
        }

    }
};

FM_ENGINE_END

#endif
