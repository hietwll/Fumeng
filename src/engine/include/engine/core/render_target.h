#ifndef FM_ENGINE_RENDER_TARGET_H
#define FM_ENGINE_RENDER_TARGET_H

#include "image.h"

FM_ENGINE_BEGIN

struct PixelRes {
    vec3 color{black};
    vec3 albedo{black};
    vec3 normal{black};
};

struct RenderTarget {
    Image m_color;
    Image m_albedo;
    Image m_normal;

    void resize(size_t width, size_t height)
    {
        m_color.resize(width, height);
        m_albedo.resize(width, height);
        m_normal.resize(width, height);
    }

    void save_to_file(const std::string& prefix, bool isHDR)
    {
        m_color.save_to_file(prefix + ".png", isHDR);
        m_albedo.save_to_file(prefix + "_albedo.png", false);
        m_normal.save_to_file(prefix + "_normal.png", false);
    }
};

FM_ENGINE_END

#endif