#include <engine/core/renderer.h>
#include <engine/core/hit_point.h>
#include <engine/core/material.h>
#include <engine/core/light.h>
#include <engine/core/render_object.h>

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
    vec3 MisEnvLight(const Scene& scene, const HitPoint& hitPoint) const;
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
                auto envLight = scene.GetEnvLight();
                if (envLight != nullptr) {
                    return envLight->GetRadiance({}, {}, {}, -ray.dir);
                }
                return black;
            }
            break;
        }

        // if first hit an emissive object
        if(idx_depth == 1)
        {
            if(hitPoint.object->IsEmissive())
            {
                auto light = hitPoint.object->GetLight();
                auto radiance = light->GetRadiance(hitPoint.pos, hitPoint.ng, hitPoint.uv, hitPoint.wo_r_w);
                color += beta * radiance;
                break;
            }
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
            direct += beta * MisEnvLight(scene, hitPoint);
        }
        color += direct / static_cast<real>(direct_loop);

        // sample BSDF
        auto bsdf_sample = hitPoint.bsdf->SampleBSDF(hitPoint.wo_r_w, sampler.Get3D());
        if(glm::length(bsdf_sample.f) < eps || bsdf_sample.pdf < eps) {
            break;
        }

        // update beta coefficient
        const real abscos = AbsDot(hitPoint.ng, bsdf_sample.wi_w);
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
    // sample the light
    auto light_sample = light->Sample(hitPoint, sampler.Get3D());

    if (light_sample.pdf < eps_pdf || glm::length(light_sample.radiance) < eps) {
        return black;
    }

    // test occlusion
    const Ray shadow_ray(light_sample.ref_pos, light_sample.wi_w, eps, light_sample.dist - eps);
    if (scene.IsIntersect(shadow_ray)) {
        return black;
    }

    // calculate bsdf
    auto bsdf_f = hitPoint.bsdf->CalFunc(hitPoint.wo_r_w, light_sample.wi_w);
    if (glm::length(bsdf_f) < eps) {
        return black;
    }

    // calculate pdf of bsdf
    auto bsdf_pdf = hitPoint.bsdf->Pdf(hitPoint.wo_r_w, light_sample.wi_w);
    if (bsdf_pdf < eps_pdf) {
        return black;
    }

    // get PowerHeuristic weight for light
    auto weight = PowerHeuristic(light_sample.pdf, bsdf_pdf);
    auto f = bsdf_f * light_sample.radiance * AbsDot(light_sample.wi_w, hitPoint.ng);
    return f / light_sample.pdf * weight;
}

/*
 * Multiple Importance Sampling for EnvLight
 */
vec3 PathTracingRenderer::MisEnvLight(const Scene& scene, const HitPoint& hitPoint) const
{
    auto envLight = scene.GetEnvLight();
    if (envLight == nullptr) {
        return black;
    }

    // sample the light
    auto light_sample = envLight->Sample(hitPoint, sampler.Get3D());

    if (light_sample.pdf < eps_pdf || glm::length(light_sample.radiance) < eps) {
        return black;
    }

    // test occlusion
    const Ray shadow_ray(light_sample.ref_pos, light_sample.wi_w, eps, light_sample.dist - eps);
    if (scene.IsIntersect(shadow_ray)) {
        return black;
    }

    // calculate bsdf
    auto bsdf_f = hitPoint.bsdf->CalFunc(hitPoint.wo_r_w, light_sample.wi_w);
    if (glm::length(bsdf_f) < eps) {
        return black;
    }

    // calculate pdf of bsdf
    auto bsdf_pdf = hitPoint.bsdf->Pdf(hitPoint.wo_r_w, light_sample.wi_w);
    if (bsdf_pdf < eps_pdf) {
        return black;
    }

    auto f = bsdf_f * light_sample.radiance * AbsDot(light_sample.wi_w, hitPoint.ng);
    auto weight = PowerHeuristic(light_sample.pdf, bsdf_pdf);
    return f / light_sample.pdf * weight;
}

/*
 * Multiple Importance Sampling for BSDF
 */
vec3 PathTracingRenderer::MisBSDF(const Scene& scene, const HitPoint& hitPoint) const
{
    // sample bsdf
    auto bsdf_sample = hitPoint.bsdf->SampleBSDF(hitPoint.wo_r_w, sampler.Get3D());
    if (glm::length(bsdf_sample.f) < eps || bsdf_sample.pdf < eps) {
        return black;
    }

    // generate new ray
    bsdf_sample.wi_w = glm::normalize(bsdf_sample.wi_w);
    const Ray r = hitPoint.GenRay(bsdf_sample.wi_w);

    // test light visibility
    HitPoint lightHit;
    auto is_intersected = scene.GetIntersect(r, &lightHit);

    // environment lighting
    if (!is_intersected) {
        auto envLight = scene.GetEnvLight();
        if (envLight == nullptr) {
            return black;
        }

        const auto radiance = envLight->GetRadiance({}, {}, {}, -r.dir);
        const vec3 f = bsdf_sample.f * radiance * AbsDot(r.dir, hitPoint.ng);

        // use bsdf sample only
        if (bsdf_sample.is_delta) {
            return f / bsdf_sample.pdf;
        }

        // use mis
        real lightPdf = envLight->Pdf(r.dir);
        auto weight = PowerHeuristic(bsdf_sample.pdf, lightPdf);
        return f / bsdf_sample.pdf * weight;
    }

    // hit something else
    if (!lightHit.object->IsEmissive()) {
        return black;
    }

    auto light_to_shd = glm::normalize(r.ori - lightHit.pos);
    auto light = lightHit.object->GetLight();
    auto radiance = light->GetRadiance(lightHit.pos, lightHit.ng, lightHit.uv, light_to_shd);

    if (glm::length(radiance) < eps) {
        return black;
    }

    auto f = bsdf_sample.f * radiance * AbsDot(hitPoint.ng, r.dir);

    /*
     * if bsdf is delta, then don't use MIS, just sample the bsdf
     * the light sampling will not work since bsdf_pdf is zero
    */
    if (bsdf_sample.is_delta) {
        return f / bsdf_sample.pdf;
    }

    auto lightPdf = light->Pdf(r.ori, lightHit.pos, lightHit.ng, light_to_shd);
    auto weight = PowerHeuristic(bsdf_sample.pdf, lightPdf);
    return f / bsdf_sample.pdf * weight;
}

SP<Renderer> CreatePathTracingRenderer(int w, int h)
{
    return MakeSP<PathTracingRenderer>(w, h);
}

FM_ENGINE_END