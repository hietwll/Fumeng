#include <engine/core/render_object.h>

FM_ENGINE_BEGIN

SP<RenderObject> CreateRenderObject(SP<const Geometry>& geometry, SP<const Material>& material, vec3 emittance = {0.0_r, 0.0_r, 0.0_r})
{
    return MakeSP<RenderObject>(geometry, material, emittance);
}

FM_ENGINE_END