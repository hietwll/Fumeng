#ifndef FUMENG_TEXTURE_H
#define FUMENG_TEXTURE_H

#include <engine/common.h>
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

class Texture
{
private:
    using WrapFunc = real(*)(real);
    WrapFunc wrap_u = &WrapClamp;
    WrapFunc wrap_v = &WrapClamp;

    virtual vec3 SampleImpl(const vec2& uv) const = 0;

public:
    static real WrapClamp(real x)
    {
        return Clamp(x, 0.0_r, 1.0_r);
    }

    static real WrapRepeat(real x)
    {
        return Clamp(x - std::floor(x), 0.0_r, 1.0_r);
    }

    Texture() = default;

    Texture(std::string& wrap_u_, std::string& wrap_v_)
    {
        InitWrap(wrap_u_, wrap_u);
        InitWrap(wrap_v_, wrap_v);
    }

    static void InitWrap(std::string& wrap_name, WrapFunc& wrap_func)
    {
        if (wrap_name == "clamp") {
            wrap_func = &WrapClamp;
        } else if (wrap_name == "repeat") {
            wrap_func = &WrapRepeat;
        } else {
            spdlog::error("invalid wrap type");
        }
    }

    virtual ~Texture() = default;

    virtual vec3 Sample(const vec2& uv) const
    {
        const real u = wrap_u(uv.x);
        const real v = wrap_u(uv.y);
        vec3 res = SampleImpl({u, v});
        return res;
    }

    virtual size_t width() const = 0;

    virtual size_t height() const = 0;
};

FM_ENGINE_END

#endif
