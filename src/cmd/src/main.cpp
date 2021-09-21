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
    SP<const Geometry> sphere = CreateSphere(1.0_r, Transform(mat4()));
    SP<const Camera> camera = CreatePinPoleCamera(vec3(0.0, 5.0_r, 0.0),
                                            vec3(0.0, -1.0_r, 0.0),
                                            vec3(0.0, 0.0, 1.0_r),
                                            5.0_r, 60.0_r, 4.0_r / 3.0_r);
    SP<const Material> material = CreateLambertDiffuse(vec3(0.0, 1.0_r, 0.0));
    SP<const RenderObject> renderObject = CreateRenderObject(sphere, material, vec3(0.0, 0.0, 0.0));

    std::vector<SP<const RenderObject>> objects;
    objects.push_back(renderObject);

    SP<const Aggregate> aggregate = CreateSimpleAggregate(objects);
    SP<Scene> scene = CreateSimpleScene(camera, aggregate);

    SP<Renderer> renderer = CreatePathTracingRenderer(800, 600);
    renderer.get()->DrawFrame(*scene.get());

    return 0;
}