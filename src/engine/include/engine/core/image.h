#ifndef FM_ENGINE_IAMGE_H
#define FM_ENGINE_IAMGE_H

#include <engine/common.h>
#include <string>

FM_ENGINE_BEGIN

class Image
{
private:
    vec3 *data_;
    size_t width_;
    size_t height_;
    size_t pixel_count_;
    const size_t channel_num_ = 3;

public:
    Image();
    Image(size_t w, size_t h);

    void resize(size_t w, size_t h);
    void fill(const vec3& val);
    void swap(Image& img);
    vec3& operator()(size_t w_idx, size_t h_idx);
    void save_to_file(const std::string& filename);
    void load_from_file(const std::string& filename);

    void destroy();
    ~Image();
};

FM_ENGINE_END

#endif