#ifndef FM_ENGINE_RENDERER_H
#define FM_ENGINE_RENDERER_H

#include <engine/common.h>
#include <engine/core/image.h>
#include <engine/core/sampler.h>
#include <engine/core/scene.h>
#include <engine/core/camera.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <thread_pool.hpp>
#include <engine/core/config.h>

FM_ENGINE_BEGIN

class RendererConfig : public Config {
private:
    static std::string root_path;

public:
    int width = 1;
    int height = 1;
    int spp = 10; // samples per pixel
    int thread_count = 8;
    std::string output = "result.png";

    static void SetRootPath(const std::string& path)
    {
        root_path = path;
    }

    void Load(const nlohmann::json &j) override
    {
        Config::Load(j);
        FM_LOAD_IMPL(j, width);
        FM_LOAD_IMPL(j, height);
        FM_LOAD_IMPL(j, spp);
        FM_LOAD_IMPL(j, thread_count);
        FM_LOAD_IMPL(j, output);
        output = root_path + output;
    }
};

class PathTracerConfig : public RendererConfig {
public:
    int depth = 10; // loop times for path tracing render
    int direct_loop = 1; // loop times for direct lighting
    int rr_depth = 10; // when to apply Russian roulette
    real rr_coef = 0.85_r; // russian roulette coefficient

    void Load(const nlohmann::json &j) override
    {
        RendererConfig::Load(j);
        FM_LOAD_IMPL(j, depth);
        FM_LOAD_IMPL(j, direct_loop);
        FM_LOAD_IMPL(j, rr_depth);
        FM_LOAD_IMPL(j, rr_coef);
    }
};

class Renderer
{
protected:
    Image m_image;
    Sampler m_sampler;
    int m_width = 1;
    int m_height = 1;
    int m_threadCount = 8;
    int m_spp = 10;
    std::string m_output = "result.png";

public:
    explicit Renderer(const RendererConfig& config) :
    m_width(config.width),
    m_height(config.height),
    m_threadCount(config.thread_count),
    m_spp(config.spp),
    m_output(config.output)
    {
        m_image.resize(m_width, m_height);
    }

    void DrawFrame(Scene& scene)
    {
        const real res_x = real(m_width);
        const real res_y = real(m_height);
        const real total_tasks = res_x * res_y;

        // create thread pool
        thread_pool threadPool(m_threadCount);

        // submit job
        for (int i = 0; i < m_width; i++)
            for (int j = 0; j < m_height; j++)
            {
                threadPool.push_task([i, j, &scene, &res_x, &res_y, this]{
                    int j_flip = m_height - 1 - j;
                    m_image(i, j_flip) = black;
                    for(int k = 0; k < m_spp; k++) {
                        const vec2 film_sample = m_sampler.Get2D();
                        const real px = (i + film_sample.x) / res_x;
                        const real py = (j + film_sample.y) / res_y;
                        Ray camera_ray = scene.GetCamera()->SampleRay({px, py});
                        m_image(i, j_flip) += RenderPixel(scene, camera_ray);
                    }
                    m_image(i, j_flip) /= m_spp;

                    Filmic(m_image(i , j_flip), 1.0_r);
                });
            }

        // check progress
        bool finished = false;
        real percent = 0.0f;
        while (!finished) {
            // check every second
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto remain_tasks = threadPool.get_tasks_total();

            // print and save
            real new_percent = 1.0 - remain_tasks / total_tasks;
            finished = remain_tasks == 0;
            spdlog::info("Current progress is : {:03.2f}", new_percent);
            if (new_percent - percent > 0.1_r || finished) {
                threadPool.paused = true;
                threadPool.wait_for_tasks();
                m_image.save_to_file(m_output, false);
                threadPool.paused = false;
                percent = new_percent;
            }
        }
    }

    virtual vec3 RenderPixel(Scene& scene, Ray& ray)
    {
        return black;
    }
};

FM_ENGINE_END

#endif