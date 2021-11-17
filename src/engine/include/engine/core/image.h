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
    bool hdr = false;

public:
    Image();
    Image(size_t w, size_t h);

    void resize(size_t w, size_t h);
    void fill(const vec3& val);
    void swap(Image& img);
    vec3& operator()(size_t w_idx, size_t h_idx);
    vec3 operator()(size_t w_idx, size_t h_idx) const;
    void save_to_file(const std::string& filename);
    void load_from_file(const std::string& filename, bool isHDR = false);
    template<typename T> void fill_data(T* raw, real scale);
    size_t width() const;
    size_t height() const;

    void destroy();
    ~Image();
};

FM_ENGINE_END

#endif