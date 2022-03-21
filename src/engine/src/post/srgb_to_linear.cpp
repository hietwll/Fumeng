#include <engine/core/post_process.h>

FM_ENGINE_BEGIN

class SrgbToLinearProcess : public ParallelPocess
{
private:
    void ProcessImpl(RenderTarget& target, size_t idx) override
    {
        target.m_color(idx).x = SrgbToLinear(target.m_color(idx).x);
        target.m_color(idx).y = SrgbToLinear(target.m_color(idx).y);
        target.m_color(idx).z = SrgbToLinear(target.m_color(idx).z);
    }
};

SP<PostProcess> CreateSrgbToLinear()
{
    return MakeSP<SrgbToLinearProcess>();
}

FM_ENGINE_END