#include <engine/create/geometry.h>
#include <engine/create/camera.h>
#include <engine/create/material.h>
#include <engine/create/render_object.h>
#include <engine/create/aggregate.h>
#include <engine/create/renderer.h>
#include <engine/create/scene.h>

#include <vector>

using namespace fumeng::engine;

int main()
{
    real aspect_ratio = 16.0 / 9.0;
    int width = 600;

    SP<const Geometry> sphere = CreateSphere(1.0_r, Transform(mat4(1.0_r)));
    SP<const Camera> camera = CreatePinPoleCamera(vec3(0.0, 15.0_r, 0.0),
                                            vec3(0.0, -1.0_r, 0.0),
                                            vec3(0.0, 0.0, 1.0_r),
                                            1.0_r, DegToRad(25.0_r), aspect_ratio);
    SP<const Material> material = CreateLambertDiffuse(vec3(1.0, 0.0_r, 0.0));
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, vec3(1.0, 1.0, 1.0));

    auto trans = Transform(vec3(3.0_r, 0.0_r, 0.0_r), black, white);
    SP<const Geometry> sphere2 = CreateSphere(1.0_r, trans);
    SP<const Material> material2 = CreateLambertDiffuse(vec3(1.0, 0.0_r, 0.0));
    auto renderObject2 = CreateRenderObject(sphere2, material2, vec3(0.0, 0.0, 0.0));

    std::vector<SP<const RenderObject>> objects;
    objects.push_back(renderObject);
    objects.push_back(renderObject2);

    SP<const Aggregate> aggregate = CreateSimpleAggregate(objects);
    SP<Scene> scene = CreateSimpleScene(camera, aggregate);

    SP<Renderer> renderer = CreatePathTracingRenderer(width, width / aspect_ratio);
    renderer.get()->DrawFrame(*scene.get());

    return 0;
}