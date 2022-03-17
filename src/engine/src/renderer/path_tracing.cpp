#include <engine/core/renderer.h>
#include <engine/core/hit_point.h>
#include <engine/core/material.h>
#include <engine/core/light.h>
#include <engine/core/render_object.h>

#include <iostream>

FM_ENGINE_BEGIN

class PathTracer : public Renderer
{
private:
    int m_depth = 10; // loop times for path tracing render
    int m_direct = 1; // loop times for direct lighting
    int m_rr_depth = 10; // when to apply Russian roulette
    real m_rr_coef = 0.85_r; // russian roulette coefficient

public:
    explicit PathTracer(const PathTracerConfig& config) :
    Renderer(config),
    m_depth(config.depth),
    m_direct(config.direct_loop),
    m_rr_depth(config.rr_depth),
    m_rr_coef(config.rr_coef)
    {
    }

    vec3 RenderPixel(Scene& scene, Ray& ray) override;
    vec3 MisLight(const Scene& scene, const Light* light, const HitPoint& hitPoint);
    vec3 MisBSDF(const Scene& scene, const HitPoint& hitPoint);
    vec3 MisEnvLight(const Scene& scene, const HitPoint& hitPoint);
};

vec3 PathTracer::RenderPixel(Scene& scene, Ray& ray)
{
    // init parameters
    vec3 color = black;
    vec3 beta = white;
    Ray r = ray;

    // main loop
    for(int idx_m_depth = 1; idx_m_depth <= m_depth; idx_m_depth++)
    {
        // find closest hit point
        HitPoint hitPoint;
        const bool hitting = scene.GetIntersect(r, &hitPoint);

        if(!hitting)
        {
            // return background color
            if (idx_m_depth == 1) {
                auto envLight = scene.GetEnvLight();
                if (envLight != nullptr) {
                    return envLight->GetRadiance({}, {}, {}, -ray.dir);
                }
                return black;
            }
            break;
        }

        // if first hit an emissive object
        if(idx_m_depth == 1)
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
        for(int i = 0; i < m_direct; i++)
        {
            for(auto& light : scene.GetLights()) {
                direct += beta * MisLight(scene, light, hitPoint);
            }
            direct += beta * MisBSDF(scene, hitPoint);
            direct += beta * MisEnvLight(scene, hitPoint);
        }
        color += direct / static_cast<real>(m_direct);

        // sample BSDF
        auto bsdf_sample = hitPoint.bsdf->SampleBSDF(hitPoint.wo_r_w, m_sampler.Get3D());
        if(glm::length(bsdf_sample.f) < eps_pdf || bsdf_sample.pdf < eps_pdf) {
            break;
        }

        // update beta coefficient
        const real abscos = AbsDot(hitPoint.ng, bsdf_sample.wi_w);
        beta *= bsdf_sample.f * abscos / bsdf_sample.pdf;

        // generate new ray
        r = hitPoint.GenRay(bsdf_sample.wi_w);

        // apply RR strategy
        if(m_depth > m_rr_depth)
        {
            if (m_sampler.Get1D() > m_rr_coef) {
                break;
            }
            beta /= m_rr_coef;
        }
    }

    return color;
}

/*
 * Multiple Importance Sampling for light
 */
vec3 PathTracer::MisLight(const Scene& scene, const Light* light, const HitPoint& hitPoint)
{
    // sample the light
    auto light_sample = light->Sample(hitPoint, m_sampler.Get3D());

    if (light_sample.pdf < eps_pdf || glm::length(light_sample.radiance) < eps_pdf) {
        return black;
    }

    // test occlusion
    const Ray shadow_ray(light_sample.ref_pos, light_sample.wi_w, eps, light_sample.dist - eps);
    if (scene.IsIntersect(shadow_ray)) {
        return black;
    }

    // calculate bsdf
    auto bsdf_f = hitPoint.bsdf->CalFunc(hitPoint.wo_r_w, light_sample.wi_w);
    if (glm::length(bsdf_f) < eps_pdf) {
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
vec3 PathTracer::MisEnvLight(const Scene& scene, const HitPoint& hitPoint)
{
    auto envLight = scene.GetEnvLight();
    if (envLight == nullptr) {
        return black;
    }

    // sample the light
    auto light_sample = envLight->Sample(hitPoint, m_sampler.Get3D());

    if (light_sample.pdf < eps_pdf || glm::length(light_sample.radiance) < eps_pdf) {
        return black;
    }

    // test occlusion
    const Ray shadow_ray(light_sample.ref_pos, light_sample.wi_w, eps, light_sample.dist - eps);
    if (scene.IsIntersect(shadow_ray)) {
        return black;
    }

    // calculate bsdf
    auto bsdf_f = hitPoint.bsdf->CalFunc(hitPoint.wo_r_w, light_sample.wi_w);
    if (glm::length(bsdf_f) < eps_pdf) {
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
vec3 PathTracer::MisBSDF(const Scene& scene, const HitPoint& hitPoint)
{
    // sample bsdf
    auto bsdf_sample = hitPoint.bsdf->SampleBSDF(hitPoint.wo_r_w, m_sampler.Get3D());
    if (glm::length(bsdf_sample.f) < eps_pdf || bsdf_sample.pdf < eps_pdf) {
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

    if (glm::length(radiance) < eps_pdf) {
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

SP<Renderer> CreatePathTracer(const PathTracerConfig& config)
{
    return MakeSP<PathTracer>(config);
}

FM_ENGINE_END