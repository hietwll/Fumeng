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
    nlohmann::json m_config;
    SP<const Camera> m_camera;
    SP<Renderer> m_render;
    SP<Scene> m_scene;

    std::optional<std::reference_wrapper<const nlohmann::json>> GetOptional(const nlohmann::json &j, const std::string& field) const
    {
        if (j.find(field) != j.end()) {
            return j.at(field);
        }
        return std::nullopt;
    }

    void ParseCamera()
    {
        auto c = GetOptional(m_config, "camera");
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

    void ParseRender()
    {
        auto c = GetOptional(m_config, "renderer");
        if (c) {
            std::string render_type = "path_tracer";
            json::LoadValue(c->get(), "type", render_type);
            if (render_type == "path_tracer") {
                PathTracerConfig config;
                config.Load(c->get());
                m_render = CreatePathTracer(config);
                return;
            }
        }

        throw std::runtime_error("Camera is not specified.");
    }

    void ParseScene()
    {
        auto c = GetOptional(m_config, "scene");
        if (c) {
            // create render objects
            auto obj_configs = GetOptional(c->get(), "render_objects");
            std::vector<SP<const RenderObject>> render_objects;
            if (obj_configs) {
                for(auto& config : obj_configs->get()) {
                    ParseRenderObjects(config, render_objects);
                }
            }

            // create aggregate
            SP<const Aggregate> aggregate;
            std::string accelerator_type = "bvh";
            auto bvh_config = GetOptional(c->get(), "bvh");
            if (bvh_config) {
                json::LoadValue(bvh_config->get(), "type", accelerator_type);
            }

            if (accelerator_type == "simple") {
                aggregate = CreateSimpleAggregate(render_objects);
            } else {
                aggregate = CreateBVHAggregate(render_objects);
            }

            // create scene
            m_scene = CreateSimpleScene(m_camera, aggregate);

            // create env
            std::string env_path;
            json::LoadValue(c->get(), "env", env_path);
            if (!env_path.empty()) {
                TextureDesc desc;
                desc.type = TextureDesc::TextureType::IMAGE;
                desc.config = MakeUP<ImageTextureConfig>(env_path);
                SP<Texture> sky = CreateTexture(desc);
                SP<EnvLight> env_light = CreateEnvLight(sky);
                m_scene->SetEnvLight(env_light);
            }
        } else  {
            throw std::runtime_error("Scene is not specified.");
        }
    }

    void ParseRenderObjects(const nlohmann::json &j, std::vector<SP<const RenderObject>>& objs)
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
                // todo: fill sphere loading
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
            json::LoadValue(material_config->get(), "type", material_type);
            if (material_type == "lambert_diffuse") {
                LambertDiffuseConfig config;
                config.Load(material_config->get());
                material = CreateLambertDiffuse(config);
            } else if (material_type == "disney") {
                DisneyConfig config;
                config.Load(material_config->get());
                material = CreateDisneyMaterial(config);
            } else {
                spdlog::error("Shape type not supported: {}.", material_type);
                throw std::runtime_error("Shape type not supported.");
            }
        }

        // create render objects
        for (auto& geom : geometries) {
            // todo: add emissive parameter in json
            objs.push_back(CreateRenderObject(geom, material, black));
        }
    }

public:
    SceneLoader() {};

    void Load(const std::string& path)
    {
        std::ifstream ifs(path);
        m_config = nlohmann::json::parse(ifs);
        ParseCamera();
        ParseRender();
        ParseScene();
    }

    void DrawFrame() const
    {
        m_render.get()->DrawFrame(*m_scene.get());
    }
};

FM_ENGINE_END

#endif
