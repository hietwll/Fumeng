#include <engine/core/post_process.h>

FM_ENGINE_BEGIN

class LinearToSrgbProcess : public ParallelPocess
{
private:
    void ProcessImpl(RenderTarget& target, size_t idx) override
    {
        target.m_color(idx).x = LinearToSrgb(target.m_color(idx).x);
        target.m_color(idx).y = LinearToSrgb(target.m_color(idx).y);
        target.m_color(idx).z = LinearToSrgb(target.m_color(idx).z);
    }
};

SP<PostProcess> CreateLinearToSrgb()
{
    return MakeSP<LinearToSrgbProcess>();
}

FM_ENGINE_END
