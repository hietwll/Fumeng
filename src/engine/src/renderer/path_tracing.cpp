#include <engine/core/renderer.h>

FM_ENGINE_BEGIN

class PathTracingRenderer : public Renderer
{
public:
    PathTracingRenderer(int w_, int h_) : Renderer(w_, h_)
    {

    }

    vec3 RenderPixel(Scene& scene, Ray& ray) const override
    {
        vec3 color = black;
        vec3 beta = vec3(1.0_r, 1.0_r, 1.0_r);
        Ray r = ray;
        if(scene.IsIntersect(r)) {
            color = vec3(1.0_r, 0.0_r, 0.0_r);
        }
        return color;
    }
};

SP<Renderer> CreatePathTracingRenderer(int w, int h)
{
    return MakeSP<PathTracingRenderer>(w, h);
}

FM_ENGINE_END