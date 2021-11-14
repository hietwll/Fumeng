#include <engine/core/distribution.h>

FM_ENGINE_BEGIN

Distribution2D::Distribution2D(vector2d& value, size_t width, size_t height)
{
    // init conditional pdf for v
    conditional_v.resize(height);
    for (int j = 0; j < height; ++j) {
        conditional_v[j] = MakeUP<Distribution1D>(value[j]);
    }
}

Sample2DInfo Distribution2D::Sample(const vec3 &sample) const
{

}

real Distribution2D::Pdf(const vec2 &uv) const
{

}

FM_ENGINE_END
