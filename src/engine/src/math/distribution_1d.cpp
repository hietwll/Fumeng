#include <engine/core/distribution.h>

FM_ENGINE_BEGIN

Distribution1D::Distribution1D(SP<const std::vector<real>> value)
{
    val = value;
    size = val->size();

    // calculate cdf
    cdf.resize(size + 1);
    cdf[0] = 0.0_r;
    for (size_t i = 1; i < size + 1; i++) {
        // here must divide size since integration is SUM(p * dx)
        cdf[i] = cdf[i - 1] + val->at(i - 1) / size;
    }

    pdf_sum = cdf[size];

    if(pdf_sum < eps_pdf) {
        spdlog::error("invalid 1d distribution.");
        return;
    }

    // normalize cdf
    for (size_t i = 0; i < size + 1; i++) {
        cdf[i] /= pdf_sum;
    }
}

size_t Distribution1D::Size() const
{
    return size;
}

Sample1DInfo Distribution1D::Sample(real sample) const
{
    const size_t low = BinarySearch(sample, cdf);
    const real delta = cdf[low + 1] - cdf[low];

    if(delta < eps_pdf || pdf_sum < eps_pdf) {
        spdlog::error("pdf is too small");
    }

    const real interp = (sample - cdf[low]) / delta;
    const real sample_val = (interp + low) / size;
    const real pdf = val->at(low) / pdf_sum;
    return {sample_val, pdf, low};
}

size_t Distribution1D::BinarySearch(real sample, const std::vector<real>& vec) const
{
    size_t left = 0;
    size_t right = vec.size() - 1;
    size_t mid = (left + right) / 2;

    while(right - left > 1) {
        if (vec[mid] <= sample) {
            left = mid;
        } else {
            right = mid;
        }
        mid = (left + right) / 2;
    }

    return left;
}

real Distribution1D::GetSum() const
{
    return pdf_sum;
}

/*
 * result need to be normalized
 */
real Distribution1D::Pdf(size_t idx) const
{
    return val->at(idx) / pdf_sum;
}

FM_ENGINE_END
