#include <engine/core/post_process.h>

FM_ENGINE_BEGIN

class ToneMapping : public ParallelPocess
{
private:
    real m_lum;

    real FilmicCore(real x)
    {
        real A = 0.22_r;
        real B = 0.30_r;
        real C = 0.10_r;
        real D = 0.20_r;
        real E = 0.01_r;
        real F = 0.30_r;
        return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    }

    void Filmic(real& value)
    {
        const static real filmicWhite = FilmicCore(11.2_r);
        value = FilmicCore(1.6_r * value * m_lum) / filmicWhite;
    }

    void ProcessImpl(RenderTarget& target, size_t idx) override
    {
        Filmic(target.m_color(idx).x);
        Filmic(target.m_color(idx).y);
        Filmic(target.m_color(idx).z);
    }

public:
    ToneMapping(const ToneMappingConfig& config) :
    m_lum(config.lum)
    {
    }
};

SP<PostProcess> CreateToneMapping(const ToneMappingConfig& config)
{
    return MakeSP<ToneMapping>(config);
}

FM_ENGINE_END
