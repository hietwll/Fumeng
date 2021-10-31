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

SP<const RenderObject> CreateObjFresnel(real radius, vec3 pos, vec3 color, real eta_in, real eta_out)
{
    SP<const Material> material = CreateSpecularReflection(color, eta_in, eta_out);
    auto trs = Transform(pos, black, white);
    SP<const Geometry> sphere = CreateSphere(radius, trs);
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, black);
    return renderObject;
}

int main()
{
    // camera
    real aspect_ratio = 1024_r / 768.0_r;
    int width = 512;
    SP<const Camera> camera = CreatePinPoleCamera(vec3 (0.0_r, 4.0_r, 0.0_r),
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

    auto mid_a = CreateObjFresnel(0.5_r, vec3(0.5_r, 0.0_r, 0.0_r), white, 1.0_r, 1.0_r);
    auto mid_b = CreateObj(0.5_r, vec3(0.5_r, 0.25_r, 1.5_r), red);

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