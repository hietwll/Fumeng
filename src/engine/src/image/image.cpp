#include <engine/core/image.h>

#include <algorithm>
#include <engine/core/utils.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


FM_ENGINE_BEGIN

Image::Image()
	: data_(nullptr), width_(0), height_(0), pixel_count_(0)
{
}

Image::Image(size_t w, size_t h)
	: data_(nullptr), width_(w), height_(h), pixel_count_(w * h)
{
	if (pixel_count_ > 0) {
		data_ = new vec3[pixel_count_];
	}
}

Image::~Image()
{
	destroy();
}

void Image::destroy()
{
	if (data_ != nullptr && pixel_count_ > 0)
	{
		delete data_;
	}
	width_ = 0;
	height_ = 0;
	pixel_count_ = 0;
}

void Image::resize(size_t w, size_t h)
{
	Image img(w, h);
	this->swap(img);
}

void Image::swap(Image& img)
{
	std::swap(data_, img.data_);
	std::swap(width_, img.width_);
	std::swap(height_, img.height_);
	std::swap(pixel_count_, img.pixel_count_);
}

vec3& Image::operator()(size_t w_idx, size_t h_idx)
{
	return data_[h_idx * width_ + w_idx];
}

vec3 Image::operator()(size_t w_idx, size_t h_idx) const
{
    return data_[h_idx * width_ + w_idx];
}

void Image::fill(const vec3& val)
{
	for (size_t i = 0; i < pixel_count_; i++)
	{
		data_[i] = val;
	}
}

void Image::save_to_file(const std::string& filename, bool isHDR, bool toSRGB)
{
    hdr = isHDR;
    if (hdr) {
        float* raw_data = new float[pixel_count_ * channel_num_];
        size_t idx = 0;
        for (size_t j = 0; j < height_; j++)
            for (size_t i = 0; i < width_; i++)
            {
                raw_data[idx++] = static_cast<float>(data_[j * width_ + i].x);
                raw_data[idx++] = static_cast<float>(data_[j * width_ + i].y);
                raw_data[idx++] = static_cast<float>(data_[j * width_ + i].z);
            }

        stbi_write_hdr(filename.c_str(), width_, height_, channel_num_, raw_data);
        delete[] raw_data;
    } else {
        uint8_t *raw_data = new uint8_t[pixel_count_ * channel_num_];
        size_t idx = 0;
        for (size_t j = 0; j < height_; j++)
            for (size_t i = 0; i < width_; i++)
            {
                // todo: LinearToSRGB, Clamp should be put in pixel process
                raw_data[idx++] = RealToUInt8(LinearToSRGB(data_[j * width_ + i].x));
                raw_data[idx++] = RealToUInt8(LinearToSRGB(data_[j * width_ + i].y));
                raw_data[idx++] = RealToUInt8(LinearToSRGB(data_[j * width_ + i].z));
            }

        stbi_write_png(filename.c_str(), width_, height_, channel_num_, raw_data, width_ * channel_num_);
        delete[] raw_data;
    }
}

template<typename T>
void Image::fill_data(T *raw, real scale)
{
    size_t idx = 0;
    for (size_t j = 0; j < height_; j++)
        for (size_t i = 0; i < width_; i++)
        {
            data_[j * width_ + i].x = static_cast<real> (raw[idx++] / scale);
            data_[j * width_ + i].y = static_cast<real> (raw[idx++] / scale);
            data_[j * width_ + i].z = static_cast<real> (raw[idx++] / scale);
        }
    delete[] raw;
}

void Image::load_from_file(const std::string& filename, bool isHDR, bool toLinear)
{
	int width, height, nrChannels;
	int desired_channels = 3;

	hdr = isHDR;

	if (hdr) {
	    float* raw_data = stbi_loadf(filename.c_str(), &width, &height, &nrChannels, desired_channels);
	    resize(width, height);
        fill_data(raw_data, 1.0_r);
	} else {
	    uint8_t* raw_data = stbi_load(filename.c_str(), &width, &height, &nrChannels, desired_channels);
	    resize(width, height);
        fill_data(raw_data, 255.0_r);

        // hdr is already in linear space
        if (toLinear) {
            to_linear();
        }
	}
}

void Image::to_srgb()
{
    apply_process(&LinearToSRGB);
}

void Image::to_linear()
{
    apply_process(&SRGBToLinear);
}

void Image::apply_process(PerElemProc func)
{
    size_t idx = 0;
    for (size_t j = 0; j < height_; j++)
        for (size_t i = 0; i < width_; i++)
        {
            data_[j * width_ + i].x = func(data_[j * width_ + i].x);
            data_[j * width_ + i].y = func(data_[j * width_ + i].y);
            data_[j * width_ + i].z = func(data_[j * width_ + i].z);
        }
}

size_t Image::width() const
{
    return width_;
}

size_t Image::height() const
{
    return height_;
}

FM_ENGINE_END