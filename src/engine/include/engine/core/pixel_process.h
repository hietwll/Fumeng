#ifndef FM_POST_PROCESS_H
#define FM_POST_PROCESS_H

#include <engine/common.h>
#include <engine/core/utils.h>
#include <engine/core/image.h>

FM_ENGINE_BEGIN

class PixelProcessImpl
{

};

class PixelProcess
{
private:
    std::vector<UP<PixelProcessImpl>>;

public:
    PixelProcess();
    void AddProcessImpl();
    void ExcutePipeline(const vec3& );
};

FM_ENGINE_END

#endif
