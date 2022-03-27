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
#include <engine/create/embree.h>
#include <engine/create/post_process.h>
#include <fstream>
#include <optional>
#include <filesystem>
#include <unordered_map>

FM_ENGINE_BEGIN

class SceneLoader {
private:
    nlohmann::json m_config;
    SP<const Camera> m_camera;
    SP<Renderer> m_render;
    SP<Scene> m_scene;
    std::vector<SP<PostProcess>> m_post_process;
    std::unordered_map<std::string, SP<const Material>> m_materials;
    std::vector<SP<const RenderObject>> m_render_objects;
    std::string m_scene_root;
    std::string m_acc_type = "bvh";

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
            } else if (camera_type == "thin_lens") {
                ThinLensCameraConfig config;
                config.Load(c->get());
                m_camera = CreateThinLensCamera(config);
                return;
            }
        }

        throw std::runtime_error("Camera is not specified.");
    }

    void ParseRender()
    {
        auto c = GetOptional(m_config, "renderer");
        if (c) {
            //register root path for output path
            RendererConfig::SetRootPath(m_scene_root);

            // parse render parameters
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
            // create aggregate
            SP<const Aggregate> aggregate;
            
            // must first load accelerator configuration
            auto acc_config = GetOptional(c->get(), "accelerator");
            if (acc_config) {
                json::LoadValue(acc_config->get(), "type", m_acc_type);
            }

            // register root path for texture loading
            ImageTextureConfig::SetRootPath(m_scene_root);

            // create materials
            auto material_configs = GetOptional(c->get(), "materials");
            if (material_configs) {
                for(auto& config : material_configs->get()) {
                    ParseMaterial(config);
                }
            }
            // add a default material
            m_materials.emplace("default_material", CreateLambertDiffuse(LambertDiffuseConfig()));

            // register root path for obj loading
            TriangleMeshConfig::SetRootPath(m_scene_root);

            // create render objects
            auto obj_configs = GetOptional(c->get(), "render_objects");
            if (obj_configs) {
                for(auto& config : obj_configs->get()) {
                    ParseRenderObject(config);
                }
            }

            if (m_acc_type == "naive") {
                aggregate = CreateSimpleAggregate(m_render_objects);
#ifdef USE_EMBREE
            } else if (m_acc_type == "embree") {
                aggregate = CreateEmbreeAggregate(m_render_objects);
#endif                
            } else {
                aggregate = CreateBVHAggregate(m_render_objects);
            }

            // create scene
            m_scene = CreateSimpleScene(m_camera, aggregate);

            // create env
            auto env_config = GetOptional(c->get(), "env");
            if (env_config) {
                std::string env_path;
                json::LoadValue(env_config->get(), "path", env_path);
                if (!env_path.empty()) {
                    TextureDesc desc;
                    desc.type = TextureDesc::TextureType::IMAGE;
                    desc.config = MakeUP<ImageTextureConfig>(m_scene_root + env_path);
                    SP<Texture> sky = CreateTexture(desc);
                    vec3 rotation {black};
                    json::LoadValue(env_config->get(), "rotation", rotation);
                    real factor {1.0_r};
                    json::LoadValue(env_config->get(), "factor", factor);
                    m_scene->AddLight( CreateEnvLight(sky, rotation, factor));
                }
            }
        } else  {
            throw std::runtime_error("Scene is not specified.");
        }
    }

    void ParseMaterial(const nlohmann::json &j)
    {
        std::string material_type;
        std::string material_name;
        SP<Material> material;
        json::LoadValue(j, "type", material_type);
        json::LoadValue(j, "name", material_name);
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
        m_materials.emplace(material_name, material);
    }

    void ParseRenderObject(const nlohmann::json &j)
    {
        // create shape
        std::vector<SP<const Geometry>> geometries;
        auto shape_config = GetOptional(j, "shape");
        if (shape_config) {
            std::string shape_type;
            json::LoadValue(shape_config->get(), "type", shape_type);
            if (shape_type == "triangle_mesh") {
#ifdef USE_EMBREE
                if (m_acc_type == "embree") {
                    TriangleMeshConfig config;
                    config.Load(shape_config->get());
                    geometries.push_back(CreateTriangleEmbree(config));
                } else
#endif
                {
                    TriangleMeshConfig config;
                    config.Load(shape_config->get());
                    CreateTriangleMesh(config, geometries);
                } 
            } else if (shape_type == "sphere") {
                SphereConfig config;
                config.Load(shape_config->get());
                geometries.push_back(CreateSphere(config));
            } else if (shape_type == "rectangle") {
                RectangleConfig config;
                config.Load(shape_config->get());
                geometries.push_back(CreateRectangle(config));
            } else {
                spdlog::error("Shape type not supported: {}.", shape_type);
                throw std::runtime_error("Shape type not supported.");
            }
        }

        // parse emittance
        vec3 emittance = black;
        json::LoadValue(j, "emittance", emittance);

        std::string material_name = "default_material";
        json::LoadValue(j, "material", material_name);

        // create render objects
        for (auto& geom : geometries) {
            if(m_materials.find(material_name) != m_materials.end()) {
                m_render_objects.push_back(CreateRenderObject(geom, m_materials[material_name], emittance));
            } else {
                spdlog::warn("Fail to find material {}, use default lambert.", material_name);
                m_render_objects.push_back(CreateRenderObject(geom, m_materials["default_material"], emittance));
            }
        }
    }

    void ParsePostProcess()
    {
        auto c = GetOptional(m_config, "post_process");
        if (c) {
            for(auto& post : c->get()) {
                std::string post_process_type;
                json::LoadValue(post, "type", post_process_type);
                if (post_process_type == "tonemapping") {
                    ToneMappingConfig config;
                    config.Load(post);
                    m_post_process.push_back(CreateToneMapping(config));
                } else if (post_process_type == "linear_to_srgb") {
                    m_post_process.push_back(CreateLinearToSrgb());
                } else if (post_process_type == "srgb_to_linear") {
                    m_post_process.push_back(CreateSrgbToLinear());
                } else if (post_process_type == "oidn_denoise") {
                    m_post_process.push_back(CreateOidnDenoise());
                }
            }
        }
    }

public:
    SceneLoader() 
    {
#ifdef USE_EMBREE
        CreateEmbreeDevice();
#endif        
    };

    ~SceneLoader()
    {
#ifdef USE_EMBREE
        DestroyEmbreeDevice();
#endif         
    }

    void Load(const std::filesystem::path& path)
    {
        m_scene_root = path.parent_path().string() + '/';
        std::ifstream ifs(path.string());
        m_config = nlohmann::json::parse(ifs);
        ParseCamera();
        ParseRender();
        ParseScene();
        ParsePostProcess();
    }

    void DrawFrame()
    {
        m_render.get()->DrawFrame(*m_scene.get(), m_post_process);
    }
};

FM_ENGINE_END

#endif
