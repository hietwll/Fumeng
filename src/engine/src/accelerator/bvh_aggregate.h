#ifndef FUMENG_BVH_AGGREGATE_H
#define FUMENG_BVH_AGGREGATE_H

#include <engine/core/aggregate.h>
#include <engine/core/render_object.h>
#include <engine/core/ray.h>

FM_ENGINE_BEGIN

struct PrimInfo
{
    RenderObject* object;
    BBox bbox;
};

struct BVHNode
{
    BBox bbox;
    // for leaf node it's start and end
    // for interior node it's left and right children
    size_t para1;
    sisz_t para2;
};

class BVHAggregate : public Aggregate
{
private:
    std::vector<SP<const RenderObject>> m_objects;
    std::vector<PrimInfo> m_primInfos;
    std::vector<BVHNode> m_nodes;
    size_t m_maxPrim = 5;

public:
    BVHAggregate(const std::vector<SP<const RenderObject>>& objects_)
    {
        objects = objects_;
    }

    void build()
    {
        if (m_objects.empty()) {
            spdlog::info("Zero object when building bvh!");
            return;
        }

        m_primInfos.resize(m_objects.size());

        for(size_t i = 0; i < m_objects.size(); i++) {
            m_primInfos[i] = {m_objects[i].get(), m_objects[i]->world_bound()};
        }
    }

};

FM_ENGINE_END

#endif //FUMENG_BVH_AGGREGATE_H
