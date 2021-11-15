#include <engine/core/distribution.h>
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

/*
 * no need to normalize value
 */
Distribution2D::Distribution2D(vector2d& value, size_t width, size_t height)
{
    // init conditional pdf
    conditional.resize(height);
    for (size_t j = 0; j < height; ++j) {
        conditional[j] = MakeUP<Distribution1D>(value[j]);
    }

    // calculate marginal data
    std::vector<real> marginal_data;
    marginal_data.resize(height);
    for (size_t j = 0; j < height; ++j) {
        marginal_data[j] = conditional[j]->GetSum();
    };

    // init marginal pdf
    marginal = MakeUP<Distribution1D>(marginal_data);
}

Sample2DInfo Distribution2D::Sample(const vec3 &sample) const
{
    // sample height (v)
    const auto res_marginal = marginal->Sample(sample.x);

    // sample width (u)
    const auto res_conditional = conditional[res_marginal.idx]->Sample(sample.y);

    // joint pdf
    const real pdf = res_marginal.pdf * res_conditional.pdf;

    return {{res_conditional.val, res_marginal.val}, pdf};
}

real Distribution2D::Pdf(const vec2 &uv) const
{
    const size_t width = conditional[0]->Size();
    const size_t height = marginal->Size();

    const size_t idx_u = Clamp(static_cast<size_t>(uv.x * width),
                               static_cast<size_t>(0), width - 1);

    const size_t idx_v = Clamp(static_cast<size_t>(uv.y * height),
                               static_cast<size_t>(0), height - 1);

    if (marginal->GetSum() < eps_pdf) {
        return 0;
    }

    return conditional[idx_v]->Pdf(idx_u) / marginal->GetSum();
}

FM_ENGINE_END
