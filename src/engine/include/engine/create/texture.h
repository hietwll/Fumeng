#ifndef FUMENG_ENGINE_CREATE_TEXTURE_H
#define FUMENG_ENGINE_CREATE_TEXTURE_H

#include <engine/core/texture.h>

FM_ENGINE_BEGIN

SP<Texture> CreateImageTexture(std::string& img_path_, std::string& wrap_u_, std::string& wrap_v_,
                               bool to_linear_, std::string& sample_name);

SP<Texture> CreateConstantTexture(const vec3& value);

FM_ENGINE_END

#endif
