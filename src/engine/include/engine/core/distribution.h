#ifndef FUMENG_DISTRIBUTION_H
#define FUMENG_DISTRIBUTION_H

#include <engine/common.h>
#include <vector>

FM_ENGINE_BEGIN

struct Sample1DInfo
{
    real val; // sampled result is between [0, 1]
    real pdf;
    size_t idx; // idx of the sampled value

    Sample1DInfo(real val_, real pdf_, size_t idx_) :
    val(val_), pdf(pdf_), idx(idx_)
    {
    }
};

struct Sample2DInfo
{
    vec2 val; // sampled result is between [0, 1]
    real pdf;
    Sample2DInfo(vec2 val_, real pdf_):
    val(val_), pdf(pdf_)
    {
    }
};

/**
 * Piecewise-Constant 1D Distribution Function
 */
class Distribution1D
{
private:
    SP<const std::vector<real>> val;
    std::vector<real> cdf;
    real pdf_sum;
    size_t size;
    size_t BinarySearch(real sample, const std::vector<real>& vec) const;

public:
    explicit Distribution1D(SP<const std::vector<real>> value);
    Sample1DInfo Sample(real sample) const;
    size_t Size() const;
    real GetSum() const;
    real Pdf(size_t idx) const;
};

/**
 * Piecewise-Constant 2D Distribution Function
 */
class Distribution2D
{
private:
    std::vector<UP<Distribution1D>> conditional;
    UP<Distribution1D> marginal;

public:
    using vector2d = std::vector<std::vector<real>>;
    Distribution2D(vector2d& value, size_t width, size_t height);
    Sample2DInfo Sample(const vec3& sample) const;
    real Pdf(const vec2& uv) const;
};


FM_ENGINE_END

#endif
