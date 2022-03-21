#ifndef FM_ENGINE_IMAGE_H
#define FM_ENGINE_IMAGE_H

#include <engine/common.h>
#include <string>

FM_ENGINE_BEGIN

class Image
{
private:
    using PerElemProc = real(*)(real);
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
    void flip_height();
    vec3& operator()(size_t w_idx, size_t h_idx);
    vec3& operator()(size_t idx);
    vec3 operator()(size_t w_idx, size_t h_idx) const;
    void save_to_file(const std::string& filename, bool isHDR = false);
    void load_from_file(const std::string& filename, bool isHDR = false, bool toLinear = true);
    void apply_process(PerElemProc func);
    void to_linear();
    void to_srgb();
    template<typename T> void fill_data(T* raw, real scale);
    size_t width() const;
    size_t height() const;
    size_t size() const;

    void destroy();
    ~Image();
};

FM_ENGINE_END

#endif