#ifndef FM_ENGINE_RENDERER_H
#define FM_ENGINE_RENDERER_H

#include <engine/common.h>
#include <engine/core/image.h>
#include <engine/core/sampler.h>
#include <engine/core/scene.h>
#include <engine/core/camera.h>

FM_ENGINE_BEGIN

class Renderer
{
private:
    Image image;
    int width = 1;
    int height = 1;
    int spp = 1;
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
                for(int k = 0; k < spp; k++) {
                    const vec2 film_sample = sampler.Get2D();
                    const real px = (i + film_sample.x) / res_x;
                    const real py = (i + film_sample.y) / res_y;
                    Ray camera_ray = scene.GetCamera()->SampleRay({px, py});
                    image(i,j) = RenderPixel(scene, camera_ray);
                }

        image.save_to_file("test.png");
    }

    virtual vec3 RenderPixel(Scene& scene, Ray& ray) const
    {
        vec3 color;
        return color;
    }
};

FM_ENGINE_END

#endif