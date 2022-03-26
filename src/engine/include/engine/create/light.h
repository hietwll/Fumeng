#ifndef FUMENG_ENGINE_CREATE_LIGHT_H
#define FUMENG_ENGINE_CREATE_LIGHT_H

#include <engine/core/light.h>

FM_ENGINE_BEGIN

SP<EnvLight> CreateEnvLight(const SP<const Texture>& texture, const vec3& rotation, real factor);

FM_ENGINE_END

#endif
