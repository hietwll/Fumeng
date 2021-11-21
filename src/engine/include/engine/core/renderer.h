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

FM_ENGINE_BEGIN

class Renderer
{
protected:
    Image image;
    int width = 1;
    int height = 1;
    int spp = 10; // samples per pixel
    int depth = 10; // loop times for path tracing render
    int direct_loop = 1; // loop times for direct lighting
    int rr_depth = 10; // when to apply Russian roulette
    real rr_coef = 0.85_r; // russian roulette coefficient
    int thread_num = 8;
    std::string output_name = "test.png";
    Sampler sampler;

public:
    Renderer(int w_, int h_) : width(w_), height(h_)
    {
        image.resize(w_, h_);
    }

    void DrawFrame(Scene& scene)
    {
        const real res_x = real(width);
        const real res_y = real(height);
        const real total_tasks = res_x * res_y;

        // create thread pool
        thread_pool threadPool(thread_num);

        // submit job
        for (int i = 0; i < width; i++)
            for (int j = 0; j < height; j++)
            {
                threadPool.push_task([i, j, &scene, &res_x, &res_y, this]{
                    int j_flip = height - 1 - j;
                    image(i, j_flip) = black;
                    for(int k = 0; k < spp; k++) {
                        const vec2 film_sample = sampler.Get2D();
                        const real px = (i + film_sample.x) / res_x;
                        const real py = (j + film_sample.y) / res_y;
                        Ray camera_ray = scene.GetCamera()->SampleRay({px, py});
                        image(i, j_flip) += RenderPixel(scene, camera_ray);
                    }
                    image(i, j_flip) /= spp;

                    Filmic(image(i ,j_flip), 1.0_r);
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
                image.save_to_file(output_name, false);
                threadPool.paused = false;
                percent = new_percent;
            }
        }
    }

    virtual vec3 RenderPixel(Scene& scene, Ray& ray) const
    {
        return black;
    }
};

FM_ENGINE_END

#endif