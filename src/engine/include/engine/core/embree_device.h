#ifdef USE_EMBREE

#ifndef FM_ENGINE_EMBREE_DEVICE_H
#define FM_ENGINE_EMBREE_DEVICE_H

#include <engine/common.h>
#include <embree3/rtcore_device.h>

FM_ENGINE_BEGIN

class EbreeDevice 
{
private:
    static RTCDevice m_device;
    EbreeDevice() = default;

public:
    static RTCDevice GetDevice();
    static void DestroyDevice();
};

FM_ENGINE_END

#endif
#endif