#include <engine/core/renderer.h>
#include <engine/core/hit_point.h>
#include <engine/core/material.h>
#include <engine/core/light.h>

#include <iostream>

FM_ENGINE_BEGIN

class PathTracingRenderer : public Renderer
{
public:
    PathTracingRenderer(int w_, int h_) : Renderer(w_, h_)
    {
    }

    vec3 RenderPixel(Scene& scene, Ray& ray) const override;
    vec3 MisLight(const Scene& scene, const Light* light, const HitPoint& hitPoint) const;
    vec3 MisBSDF(const Scene& scene, const HitPoint& hitPoint) const;
};

vec3 PathTracingRenderer::RenderPixel(Scene& scene, Ray& ray) const
{
    // init parameters
    vec3 color = black;
    vec3 beta = white;
    Ray r = ray;

    // main loop
    for(int idx_depth = 1; idx_depth <= depth; idx_depth++)
    {
        // find closest hit point
        HitPoint hitPoint;
        const bool hitting = scene.GetIntersect(r, &hitPoint);

        if(!hitting)
        {
            // return background color
            if (idx_depth == 1) {
                return white;
            }
            break;
        }

        // get material's bsdf
        hitPoint.material->CreateBSDF(hitPoint);

        // direct lighting
        vec3 direct = black;
        for(int i = 0; i < direct_loop; i++)
        {
            for(auto& light : scene.GetLights()) {
                direct += beta * MisLight(scene, light, hitPoint);
            }
            direct += beta * MisBSDF(scene, hitPoint);
        }

        color += direct / static_cast<real>(direct_loop);

        // sample BSDF
        auto bsdf_sample = hitPoint.bsdf->SampleBSDF(-r.dir, sampler.Get2D());
        if(bsdf_sample.f.length() < eps || bsdf_sample.pdf < eps) {
            break;
        }

        // update beta coefficient
        const real abscos = std::abs(glm::dot(hitPoint.ng, bsdf_sample.wi_w));
        beta *= bsdf_sample.f * abscos / bsdf_sample.pdf;

        // generate new ray
        r = hitPoint.GenRay(bsdf_sample.wi_w);

        // apply RR strategy
        if(depth > rr_depth)
        {
            if (sampler.Get1D() > rr_coef) {
                break;
            }
            beta /= rr_coef;
        }
    }
    return color;
}

/*
 * Multiple Importance Sampling for light
 */
vec3 PathTracingRenderer::MisLight(const Scene& scene, const Light* light, const HitPoint& hitPoint) const
{

}

/*
 * Multiple Importance Sampling for BSDF
 */
vec3 PathTracingRenderer::MisBSDF(const Scene& scene, const HitPoint& hitPoint) const
{
}

SP<Renderer> CreatePathTracingRenderer(int w, int h)
{
    return MakeSP<PathTracingRenderer>(w, h);
}

FM_ENGINE_END