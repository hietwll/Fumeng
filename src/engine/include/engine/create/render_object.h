#ifndef FM_ENGINE_CREATE_RENDER_OBJECT_H
#define FM_ENGINE_CREATE_RENDER_OBJECT_H

#include <engine/core/render_object.h>

FM_ENGINE_BEGIN

SP<RenderObject> CreateRenderObject(SP<const Geometry>& geometry, SP<const Material>& material, vec3 emittance = {0.0_r, 0.0_r, 0.0_r});

FM_ENGINE_END

#endif