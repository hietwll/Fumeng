#include <engine/core/texture.h>
#include <engine/core/image.h>

FM_ENGINE_BEGIN

class ImageTexture : public Texture
{
private:
    using SampleFunc = vec3(*)(const vec2& uv, const Image& img);
    Image m_image;
    SampleFunc m_sampler = &NearestSample;

    static vec3 LinearSample(const vec2& uv, const Image& img)
    {
        const auto ru = uv.x * static_cast<real>(img.width());
        const auto rv = uv.y * static_cast<real>(img.height());

        const size_t u0 = static_cast<size_t>(Clamp(ru - 0.5_r, 0.0_r, static_cast<real>(img.width() - 1)));
        const size_t u1 = Clamp(u0 + 1, size_t(0), img.width() - 1);
        const real pu = Clamp(ru - 0.5_r - u0, 0.0_r, 1.0_r);

        const size_t v0 = static_cast<size_t>(Clamp(rv - 0.5_r, 0.0_r, static_cast<real>(img.height() - 1)));
        const size_t v1 = Clamp(v0 + 1, size_t(0), img.height() - 1);
        const real pv = Clamp(rv - 0.5_r - v0, 0.0_r, 1.0_r);

        const vec3 u0v0 = img(u0, v0);
        const vec3 u1v0 = img(u1, v0);
        const vec3 u0v1 = img(u0, v1);
        const vec3 u1v1 = img(u1, v1);

        return (u0v0 * (1 - pu) + u1v0 * pu) * (1 - pv) + (u0v1 * (1 - pu) + u1v1 * pu) * pv;
    }

    static vec3 NearestSample(const vec2& uv, const Image& img)
    {
        const auto ru = static_cast<size_t>(uv.x * static_cast<real>(img.width()));
        const auto rv = static_cast<size_t>(uv.y * static_cast<real>(img.height()));

        const size_t u = Clamp(ru, size_t(0) ,img.width() - 1);
        const size_t v = Clamp(rv, size_t(0) ,img.height() - 1);
        return img(u, v);
    }

    static void InitSampler(const std::string& sample_name, SampleFunc & sample_func)
    {
        if (sample_name == "nearest") {
            sample_func = &NearestSample;
        } else if (sample_name == "linear") {
            sample_func = &LinearSample;
        } else {
            spdlog::error("invalid sample type");
        }
    }

    vec3 SampleImpl(const vec2& uv) const override
    {
        return m_sampler(uv, m_image);
    }

public:
    ImageTexture(const ImageTextureConfig& config) : Texture(config)
    {
        bool isHDR = false;
        if (config.path.find(".hdr") != std::string::npos) {
            isHDR = true;
        }
        m_image.load_from_file(config.path, isHDR);
        InitSampler(config.sampler, m_sampler);
    }

    size_t width() const override
    {
        return m_image.width();
    }

    size_t height() const override
    {
        return m_image.height();
    }

};

SP<Texture> CreateImageTexture(const ImageTextureConfig& config)
{
    return MakeSP<ImageTexture>(config);
}

FM_ENGINE_END
