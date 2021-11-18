#ifndef FUMENG_ENGINE_CREATE_LIGHT_H
#define FUMENG_ENGINE_CREATE_LIGHT_H

#include <engine/core/light.h>

FM_ENGINE_BEGIN

SP<Light> CreateEnvLight(const SP<const Texture>& texture_);

FM_ENGINE_END

#endif
