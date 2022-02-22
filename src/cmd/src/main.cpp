#include <engine/create/geometry.h>
#include <engine/create/camera.h>
#include <engine/create/material.h>
#include <engine/create/render_object.h>
#include <engine/create/aggregate.h>
#include <engine/create/renderer.h>
#include <engine/create/scene.h>
#include <engine/create/texture.h>
#include <engine/create/light.h>

#include <vector>

using namespace fumeng::engine;

SP<const RenderObject> CreateObj(real radius, const vec3& pos, const vec3& diffuse_color, const vec3& emission = black)
{
    SP<const Material> material = CreateLambertDiffuse(CreateConstantTexture(diffuse_color));
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, emission);
    return renderObject;
}

SP<const RenderObject> CreateObj(real radius, const vec3& pos, const SP<Texture>& color, const vec3& emission = black)
{
    SP<const Material> material = CreateLambertDiffuse(color);
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, emission);
    return renderObject;
}

std::vector<SP<const RenderObject>> CreateDisneyMesh(
        const std::string filename,
        const vec3& emission = black,
        const vec3& basecolor = vec3(1.0_r, 0.0_r, 0.0_r),
        real metallic = 0.0_r,
        real specular = 0.0_r,
        real specularTint = 0.0_r,
        real roughness = 0.0_r,
        real anisotropic = 0.0_r,
        real sheen = 0.0_r,
        real sheenTint = 0.0_r,
        real clearcoat = 0.0_r,
        real clearcoatGloss = 0.0_r,
        real specTrans = 0.0_r,
        real specTransRoughness = 0.0_r,
        real diffTrans = 1.0_r,
        real flatness = 0.0_r,
        real ior = 1.5_r,
        real thin = false)
{
    SP<Texture> basecolor_tex = CreateConstantTexture(basecolor);
    SP<Texture> metallic_tex = CreateConstantTexture(metallic);
    SP<Texture> specular_tex = CreateConstantTexture(specular);
    SP<Texture> specularTint_tex = CreateConstantTexture(specularTint);
    SP<Texture> roughness_tex = CreateConstantTexture(roughness);
    SP<Texture> anisotropic_tex = CreateConstantTexture(anisotropic);
    SP<Texture> sheen_tex = CreateConstantTexture(sheen);
    SP<Texture> sheenTint_tex = CreateConstantTexture(sheenTint);
    SP<Texture> clearcoat_tex = CreateConstantTexture(clearcoat);
    SP<Texture> clearcoatGloss_tex = CreateConstantTexture(clearcoatGloss);
    SP<Texture> specTrans_tex = CreateConstantTexture(specTrans);
    SP<Texture> specTransRoughness_tex = CreateConstantTexture(specTransRoughness);
    SP<Texture> diffTrans_tex = CreateConstantTexture(diffTrans);
    SP<Texture> flatness_tex = CreateConstantTexture(flatness);
    SP<Texture> ior_tex = CreateConstantTexture(ior);

    SP<const Material> material = CreateDisneyMaterial(
            basecolor_tex,
            metallic_tex,
            specular_tex,
            specularTint_tex,
            roughness_tex,
            anisotropic_tex,
            sheen_tex,
            sheenTint_tex,
            clearcoat_tex,
            clearcoatGloss_tex,
            specTrans_tex,
            specTransRoughness_tex,
            diffTrans_tex,
            flatness_tex,
            ior_tex,
            thin);

    auto triangles = CreateTriangleMesh(filename);
    std::vector<SP<const RenderObject>> objects;
    for(auto& tri : triangles) {
        objects.push_back(CreateRenderObject(tri, material, black));
    }
    return objects;
}

SP<const RenderObject> CreateDisneyObj(real radius,
                                       const vec3& pos,
                                       const vec3& emission = black,
                                       const vec3& basecolor = red,
                                       real metallic = 1.0_r,
                                       real specular = 0.0_r,
                                       real specularTint = 0.0_r,
                                       real roughness = 0.0_r,
                                       real anisotropic = 0.0_r,
                                       real sheen = 0.0_r,
                                       real sheenTint = 0.0_r,
                                       real clearcoat = 0.0_r,
                                       real clearcoatGloss = 0.0_r,
                                       real specTrans = 0.0_r,
                                       real specTransRoughness = 0.0_r,
                                       real diffTrans = 0.0_r,
                                       real flatness = 0.0_r,
                                       real ior = 1.5_r,
                                       real thin = false)
{
    SP<Texture> basecolor_tex = CreateConstantTexture(basecolor);
    SP<Texture> metallic_tex = CreateConstantTexture(metallic);
    SP<Texture> specular_tex = CreateConstantTexture(specular);
    SP<Texture> specularTint_tex = CreateConstantTexture(specularTint);
    SP<Texture> roughness_tex = CreateConstantTexture(roughness);
    SP<Texture> anisotropic_tex = CreateConstantTexture(anisotropic);
    SP<Texture> sheen_tex = CreateConstantTexture(sheen);
    SP<Texture> sheenTint_tex = CreateConstantTexture(sheenTint);
    SP<Texture> clearcoat_tex = CreateConstantTexture(clearcoat);
    SP<Texture> clearcoatGloss_tex = CreateConstantTexture(clearcoatGloss);
    SP<Texture> specTrans_tex = CreateConstantTexture(specTrans);
    SP<Texture> specTransRoughness_tex = CreateConstantTexture(specTransRoughness);
    SP<Texture> diffTrans_tex = CreateConstantTexture(diffTrans);
    SP<Texture> flatness_tex = CreateConstantTexture(flatness);
    SP<Texture> ior_tex = CreateConstantTexture(ior);

    SP<const Material> material = CreateDisneyMaterial(
            basecolor_tex,
            metallic_tex,
            specular_tex,
            specularTint_tex,
            roughness_tex,
            anisotropic_tex,
            sheen_tex,
            sheenTint_tex,
            clearcoat_tex,
            clearcoatGloss_tex,
            specTrans_tex,
            specTransRoughness_tex,
            diffTrans_tex,
            flatness_tex,
            ior_tex,
            thin);
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, emission);
    return renderObject;
}

SP<const RenderObject> CreateMirror(real radius, const vec3& pos, const vec3& color, const vec3& eta_i, const vec3& eta_t, const vec3& k)
{
    SP<const Material> material = CreateSpecularReflection(color, eta_i, eta_t, k);
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, black);
    return renderObject;
}

SP<const RenderObject> CreateGlass(real radius, const vec3& pos, const vec3& color, const real eta_i, const real eta_t)
{
    SP<const Material> material = CreateSpecularTransmission(color, color, eta_i, eta_t);
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, black);
    return renderObject;
}

SP<Texture> CreateTexture(std::string path)
{
    std::string wrap_mode = "repeat";
    std::string sample_mode = "linear";

    return CreateImageTexture(path, wrap_mode, wrap_mode, sample_mode);
}

int main()
{
    // camera
    real aspect_ratio = 1024_r / 512.0_r;
    int width = 1024;
    SP<const Camera> camera = CreatePinPoleCamera(vec3 (-8.0_r, 0.0_r, 0.0_r),
                                                  vec3(1.0_r, 0.0_r, 0.0_r),
                                            vec3(0.0, 0.0, 1.0_r),
                                            1.0_r, DegToRad(30.0_r), aspect_ratio);

    // objects
    std::vector<SP<const RenderObject>> objects;

    // textures
    SP<Texture> earth = CreateTexture("earth.png");
    SP<Texture> sky = CreateTexture("park_4k.hdr");
    SP<EnvLight> envLight = CreateEnvLight(sky);

    // left
    auto left = CreateObj(1e5_r, vec3(-1e5_r - 2.0_r, 0.0_r, 0.0_r), vec3(0.75_r, 0.25_r, 0.25_r));
    auto right = CreateObj(1e5_r, vec3(1e5_r + 2.0_r, 0.0_r, 0.0_r), vec3(0.25_r, 0.25_r, 0.75_r));
    auto back = CreateObj(1e5_r, vec3(0.0_r, -1e5_r - 2.0_r, 0.0_r), vec3(0.75_r, 0.75_r, 0.75_r));
    auto front = CreateObj(1e5_r, vec3(0.0_r, 1e5_r + 10.0_r, 0.0_r), vec3(1.0_r, 1.0_r, 1.0_r));
    auto bottom = CreateObj(1e5_r, vec3(0.0_r, 0.0_r, -1e5_r - 2.0_r), vec3(0.75_r, 0.75_r, 0.75_r));
    auto top = CreateObj(1e5_r, vec3(0.0_r, 0.0_r, 1e5_r + 2.0_r), vec3(0.75_r, 0.75_r, 0.75_r));

    auto mid_a = CreateDisneyObj(0.5_r, vec3(0.0_r, 0.0_r, 0.0_r), black, 0.9_r * red);
//    auto mid_b = CreateMirror(0.5_r, vec3(10.0_r, 0.0_r, -1.0_r), vec3(0.99_r), vec3(1.0_r), vec3(1.5_r), white);

    auto light = CreateObj(3.0_r, vec3(0.0_r, -5.0_r, 8.0_r), white, white * 10.0_r);



//    objects.push_back(left);
//    objects.push_back(right);
//    objects.push_back(back);
//    objects.push_back(front);
//
//    objects.push_back(bottom);
//    objects.push_back(top);

    objects.push_back(mid_a);
//    objects.push_back(mid_b);
//    objects.push_back(light);

    auto triObjs = CreateDisneyMesh("DamagedHelmet.obj");

    SP<const Aggregate> aggregate = CreateBVHAggregate(triObjs);
//    SP<const Aggregate> aggregate = CreateSimpleAggregate(triObjs);
    SP<Scene> scene = CreateSimpleScene(camera, aggregate);

    scene->SetEnvLight(envLight);

    SP<Renderer> renderer = CreatePathTracingRenderer(width, int(real(width) / aspect_ratio));
    renderer.get()->DrawFrame(*scene.get());

    return 0;
}