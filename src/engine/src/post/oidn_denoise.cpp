#ifdef USE_EMBREE

#include <engine/core/post_process.h>
#include <OpenImageDenoise/oidn.hpp>

FM_ENGINE_BEGIN

class OidnDenoise : public PostProcess
{
private:
    void CopyFromImage(real* data, Image& image)
    {
        for (int j = 0; j < image.height(); ++j) {
            for (int i = 0; i < image.width(); ++i) {
                int idx = j * image.width() + i;
                data[idx * 3 + 0] = image(i, j).x;
                data[idx * 3 + 1] = image(i, j).y;
                data[idx * 3 + 2] = image(i, j).z;
            }
        }
    }

    void CopyToImage(real* data, Image& image)
    {
        for (int j = 0; j < image.height(); ++j) {
            for (int i = 0; i < image.width(); ++i) {
                int idx = j * image.width() + i;
                image(i, j).x = data[idx * 3 + 0];
                image(i, j).y = data[idx * 3 + 1];
                image(i, j).z = data[idx * 3 + 2];
            }
        }
    }

public:
    void Process(RenderTarget& target) override
    {
        // Create an Intel Open Image Denoise device
        oidn::DeviceRef device = oidn::newDevice();
        device.commit();

        const size_t width = target.m_color.width();
        const size_t height = target.m_color.height();

        real* color = new real[width * height * 3];
        real* albedo = new real[width * height * 3];
        real* normal = new real[width * height * 3];
        real* output = new real[width * height * 3];

        CopyFromImage(color, target.m_color);
        CopyFromImage(albedo, target.m_albedo);
        CopyFromImage(normal, target.m_normal);

        // Create a filter for denoising a beauty (color) image using optional auxiliary images too
        oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
        filter.setImage("color",  color,  oidn::Format::Float3, width, height); // beauty
        filter.setImage("albedo", albedo, oidn::Format::Float3, width, height); // auxiliary
        filter.setImage("normal", normal, oidn::Format::Float3, width, height); // auxiliary
        filter.setImage("output", output, oidn::Format::Float3, width, height); // denoised beauty
        filter.set("hdr", true); // beauty image is HDR
        filter.commit();

        // Filter the image
        filter.execute();

        // Check for errors
        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None) {
            spdlog::error(" Error: {}", errorMessage);
        }

        CopyToImage(output, target.m_color);

        delete[] color;
        delete[] albedo;
        delete[] normal;
        delete[] output;
    }
};

SP<PostProcess> CreateOidnDenoise()
{
    return MakeSP<OidnDenoise>();
}

FM_ENGINE_END

#endif

