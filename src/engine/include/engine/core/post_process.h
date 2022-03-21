#ifndef FM_ENGINE_POST_PROCESS_H
#define FM_ENGINE_POST_PROCESS_H

#include <engine/common.h>
#include <engine/core/utils.h>
#include <engine/core/config.h>
#include "render_target.h"

FM_ENGINE_BEGIN

class ToneMappingConfig : public Config
{
public:
    real lum {1.0_r};

    void Load(const nlohmann::json &j) override
    {
        FM_LOAD_IMPL(j, lum);
    }
};

class PostProcess
{
public:
    virtual ~PostProcess() = default;
    virtual void Process(RenderTarget& target) = 0;
};

class ParallelPocess : public PostProcess
{
private:
    virtual void ProcessImpl(RenderTarget& target, size_t idx) = 0;

public:
    virtual void Process(RenderTarget& target)
    {
        // https://stackoverflow.com/questions/63340193/c-how-to-elegantly-use-c17-parallel-execution-with-for-loop-that-counts-an-i
//        std::vector<size_t> v(target.m_color.size());
//        std::iota(std::begin(v), std::end(v), 0);
//        std::for_each(std::execution::par_unseq, std::begin(v), std::end(v), [&](int i) {
//            ProcessImpl(target, i);
//        });

        //todo: use parallel for
        for (int i = 0; i < target.m_color.size(); ++i) {
            ProcessImpl(target, i);
        }
    }
};

FM_ENGINE_END

#endif
