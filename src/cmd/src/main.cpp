#include <engine/create/geometry.h>
#include <engine/create/camera.h>
#include <engine/create/material.h>
#include <engine/create/render_object.h>
#include <engine/create/aggregate.h>
#include <engine/create/renderer.h>
#include <engine/create/scene.h>

#include <vector>

using namespace fumeng::engine;

SP<const RenderObject> CreateObj(real radius, vec3 pos, vec3 diffuse_color,vec3 emission = black)
{
    SP<const Material> material = CreateLambertDiffuse(diffuse_color);
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

int main()
{
    // camera
    real aspect_ratio = 1024_r / 768.0_r;
    int width = 1024;
    SP<const Camera> camera = CreatePinPoleCamera(vec3 (0.0_r, 8.0_r, 0.0_r),
                                                  vec3(0.0_r, -1.0_r, 0.0_r),
                                            vec3(0.0, 0.0, 1.0_r),
                                            1.0_r, DegToRad(45.0_r), aspect_ratio);

    // objects
    std::vector<SP<const RenderObject>> objects;

    // left
    auto left = CreateObj(1e5_r, vec3(-1e5_r - 2.0_r, 0.0_r, 0.0_r), vec3(0.75_r, 0.25_r, 0.25_r));
    auto right = CreateObj(1e5_r, vec3(1e5_r + 2.0_r, 0.0_r, 0.0_r), vec3(0.25_r, 0.25_r, 0.75_r));
    auto back = CreateObj(1e5_r, vec3(0.0_r, -1e5_r - 2.0_r, 0.0_r), vec3(0.75_r, 0.75_r, 0.75_r));
    auto front = CreateObj(1e5_r, vec3(0.0_r, 1e5_r + 10.0_r, 0.0_r), vec3(1.0_r, 1.0_r, 1.0_r));
    auto bottom = CreateObj(1e5_r, vec3(0.0_r, 0.0_r, -1e5_r - 2.0_r), vec3(0.75_r, 0.75_r, 0.75_r));
    auto top = CreateObj(1e5_r, vec3(0.0_r, 0.0_r, 1e5_r + 2.0_r), vec3(0.75_r, 0.75_r, 0.75_r));

    auto mid_a = CreateMirror(0.5_r, vec3(0.5_r, 0.25_r, 1.5_r),
                                  vec3(0.9, 0.9, 0.9), white,
                                  vec3(1.4_r, 1.4_r, 1.4_r),
                                  vec3(3.9_r, 3.9_r, 3.9_r));
    auto mid_b = CreateGlass(0.5_r, vec3(-1.0_r, -1.0_r, 1.5_r), vec3(0.93_r), 1.0_r, 1.5_r);

    auto light = CreateObj(2.0_r, vec3(0.0_r, 0.0_r, -3.732_r), white, white * 10.0_r);

    objects.push_back(left);
    objects.push_back(right);
    objects.push_back(back);
    objects.push_back(front);

    objects.push_back(bottom);
    objects.push_back(top);

    objects.push_back(mid_a);
    objects.push_back(mid_b);
    objects.push_back(light);

    SP<const Aggregate> aggregate = CreateSimpleAggregate(objects);
    SP<Scene> scene = CreateSimpleScene(camera, aggregate);

    SP<Renderer> renderer = CreatePathTracingRenderer(width, int(real(width) / aspect_ratio));
    renderer.get()->DrawFrame(*scene.get());

    return 0;
}