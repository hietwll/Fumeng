#ifndef FM_ENGINE_CREATE_RENDERER_H
#define FM_ENGINE_CREATE_RENDERER_H

#include <engine/core/renderer.h>

FM_ENGINE_BEGIN

SP<Renderer> CreatePathTracingRenderer(int w, int h);

FM_ENGINE_END

#endif