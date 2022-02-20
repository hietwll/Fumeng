#ifndef FM_ENGINE_BBOX_H
#define FM_ENGINE_BBOX_H

#include <engine/common.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>

FM_ENGINE_BEGIN

class BBox
{
public:
    vec3 m_min;
    vec3 m_max;

    BBox() noexcept :
    m_min(REAL_MAX), m_max(REAL_MIN)
    {
    }

    BBox(const vec3& min, const vec3& max) noexcept :
    m_min(min), m_max(max)
    {
    }

    BBox& operator|=(const vec3& p)
    {
        for(int i = 0 ; i < 3; i++){
            m_min[i] = std::min(p[i], m_min[i]);
            m_max[i] = std::max(p[i], m_max[i]);
        }
        return *this;
    }

    BBox& operator|=(const BBox& box)
    {
        for(int i = 0 ; i < 3; i++){
            m_min[i] = std::min(box.m_min[i], m_min[i]);
            m_max[i] = std::max(box.m_max[i], m_max[i]);
        }
        return *this;
    }

    int MaxSpanAxis() const
    {
        real max_len = 0_r;
        int axis = 0;
        for(int i = 0; i < 3; ++i) {
            const real axis_len = m_max[i] - m_min[i];

            if(axis_len > max_len) {
                max_len = axis_len;
                axis = i;
            }
        }
        return axis;
    }

    bool IsIntersect(const Ray& r, const vec3& dir_inv) const
    {
        const vec3 low = dir_inv * (m_min - r.ori);
        const vec3 hig = dir_inv * (m_max - r.ori);

        const vec3 min_lh = MinVec(low, hig);
        const vec3 max_lh = MaxVec(low, hig);

        return MaxElem(MaxVec(vec3(r.t_min), min_lh)) <=
               MinElem(MinVec(vec3(r.t_max), max_lh));
    }
};

inline BBox operator|(const BBox& lhs, const BBox& rhs)
{
    return BBox(vec3(std::min(lhs.m_min.x, rhs.m_min.x),
                     std::min(lhs.m_min.y, rhs.m_min.y),
                     std::min(lhs.m_min.z, rhs.m_min.z)),
                vec3(std::max(lhs.m_max.x, rhs.m_max.x),
                     std::max(lhs.m_max.y, rhs.m_max.y),
                     std::max(lhs.m_max.z, rhs.m_max.z)));
}

FM_ENGINE_END

#endif //FM_ENGINE_BBOX_H
