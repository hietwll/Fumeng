#ifdef USE_EMBREE

#include <engine/core/embree_device.h>

FM_ENGINE_BEGIN

RTCDevice EbreeDevice::m_device = nullptr;

RTCDevice EbreeDevice::GetDevice()
{
    if (m_device == nullptr) {
        m_device = rtcNewDevice(nullptr);
    }
    return m_device;
}


void EbreeDevice::DestroyDevice()
{
    rtcReleaseDevice(m_device);
}


void CreateEmbreeDevice()
{
    EbreeDevice::GetDevice();
}

void DestroyEmbreeDevice()
{
    EbreeDevice::DestroyDevice();
}

FM_ENGINE_END

#endif