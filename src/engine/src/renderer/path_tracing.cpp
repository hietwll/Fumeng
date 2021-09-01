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

    }
};

FM_ENGINE_END