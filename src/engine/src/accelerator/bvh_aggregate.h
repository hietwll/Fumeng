#ifndef FUMENG_BVH_AGGREGATE_H
#define FUMENG_BVH_AGGREGATE_H

#include <engine/core/aggregate.h>
#include <engine/core/render_object.h>
#include <engine/core/ray.h>

FM_ENGINE_BEGIN

struct ObjInfo
{
    RenderObject* object;
    BBox box;
};

struct BVHNode
{
    BBox box;
    // for leaf node it's begin and end
    // for interior node it's left and right children
    size_t para1;
    sisz_t para2;
};

class BVHAggregate : public Aggregate
{
private:
    std::vector<SP<const RenderObject>> m_objects;
    std::vector<ObjInfo> m_objInfos;
    std::vector<BVHNode> m_nodes;
    std::vector<RenderObject*> m_objList;
    size_t m_leafMax = 5;

public:
    BVHAggregate(const std::vector<SP<const RenderObject>>& objects)
    {
        m_objects = objects;
    }

    void build()
    {
        if (m_objects.empty()) {
            spdlog::info("Zero object when building bvh!");
            return;
        }

        m_objInfos.resize(m_objects.size());
        m_objList.reserve(m_objects.size());

        for(size_t i = 0; i < m_objects.size(); i++) {
            m_objInfos[i] = {m_objects[i].get(), m_objects[i]->WorldBound()};
        }

        // recursively build tree in range [0, end)
        build_impl(m_objInfos, 0, m_objInfos.size());
    }

    void build_impl(vector<ObjInfo>& infos, const size_t start, const size_t end)
    {
        const size_t size = end - start;

        if (size <= m_leafMax) {
            BBox box;
            for(size_t i = start; i < end; i++) {
                m_objList.push_back(infos[i].object);
                box |= infos[i].box;
            }
            m_nodes.push_back(BVHNode{std::move(box), start, end});
            return;
        }

        // find the max span axis
        BBox box_root;
        for(size_t i = start; i < end; i++) {
            box_root |= infos[i].box;
        }

        size_t split_axis = box_root.MaxSpanAxis();
    }

};

FM_ENGINE_END

#endif //FUMENG_BVH_AGGREGATE_H
