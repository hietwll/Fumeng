#ifndef FM_ENGINE_CREATE_POST_PROCESS_H
#define FM_ENGINE_CREATE_POST_PROCESS_H

#include <engine/core/post_process.h>

FM_ENGINE_BEGIN

SP<PostProcess> CreateSrgbToLinear();
SP<PostProcess> CreateLinearToSrgb();
SP<PostProcess> CreateToneMapping(const ToneMappingConfig& config);
SP<PostProcess> CreateOidnDenoise();

FM_ENGINE_END

#endif