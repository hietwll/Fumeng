#include <engine/core/image.h>

#include <algorithm>

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

void Image::fill(const vec3& val)
{
	for (size_t i = 0; i < pixel_count_; i++)
	{
		data_[i] = val;
	}
}

void Image::save_to_file(const std::string& filename)
{
	uint8_t *raw_data = new uint8_t[pixel_count_ * channel_num_];
	size_t idx = 0;
	for (size_t j = 0; j < height_; j++)
		for (size_t i = 0; i < width_; i++)
		{
			raw_data[idx++] = static_cast<uint8_t> (255.0 * data_[j * width_ + i].x);
			raw_data[idx++] = static_cast<uint8_t> (255.0 * data_[j * width_ + i].y);
			raw_data[idx++] = static_cast<uint8_t> (255.0 * data_[j * width_ + i].z);
		}

	stbi_write_png(filename.c_str(), width_, height_, channel_num_, raw_data, width_ * channel_num_);
	delete[] raw_data;
}

void Image::load_from_file(const std::string& filename)
{
	int width, height, nrChannels;
	int desired_channels = 3;
	uint8_t* raw_data = stbi_load(filename.c_str(), &width, &height, &nrChannels, desired_channels);

	resize(width, height);
	size_t idx = 0;
	for (size_t j = 0; j < height_; j++)
		for (size_t i = 0; i < width_; i++)
		{
			data_[j * width_ + i].x = static_cast<real> (raw_data[idx++] / 255.0);
			data_[j * width_ + i].y = static_cast<real> (raw_data[idx++] / 255.0);
			data_[j * width_ + i].z = static_cast<real> (raw_data[idx++] / 255.0);
		}
}

FM_ENGINE_END