#ifndef FM_ENGINE_RENDERER_H
#define FM_ENGINE_RENDERER_H

#include <engine/common.h>
#include <engine/core/image.h>
#include <engine/core/sampler.h>
#include <engine/core/scene.h>
#include <engine/core/camera.h>
#include <engine/core/ray.h>

FM_ENGINE_BEGIN

class Renderer
{
protected:
    Image image;
    int width = 1;
    int height = 1;
    int spp = 100; // samples per pixel
    int depth = 1; // loop times for path tracing render
    int direct_loop = 10; // loop times for direct lighting
    int rr_depth = 10; // when to apply Russian roulette
    real rr_coef = 0.65_r; // russian roulette coefficient

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

        for (int i = 0; i < width; i++)
            for (int j = 0; j < height; j++)
            {
                image(i,j) = black;
                for(int k = 0; k < spp; k++) {
                    const vec2 film_sample = sampler.Get2D();
                    const real px = (i + film_sample.x) / res_x;
                    const real py = (j + film_sample.y) / res_y;
                    Ray camera_ray = scene.GetCamera()->SampleRay({px, py});
                    //Ray test_ray(vec3(0.0_r, 5.0_r, 0.0_r), vec3(0.0_r, -1.0_r, 0.0_r));
                    image(i,j) += RenderPixel(scene, camera_ray);
                }
                image(i,j) /= spp;
            }
        image.save_to_file("test.png");
    }

    virtual vec3 RenderPixel(Scene& scene, Ray& ray) const
    {
        return black;
    }
};

FM_ENGINE_END

#endif