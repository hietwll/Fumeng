#include <engine/core/aggregate.h>
#include <engine/core/render_object.h>
#include <engine/core/ray.h>

FM_ENGINE_BEGIN
namespace {
    struct ObjInfo
    {
        const RenderObject* object;
        BBox box;
    };

    struct BVHNode
    {
        BBox box;
        bool is_leaf;
        // leaf node: idx of begin and end primitives in m_objList
        // interior node: idx of left and right children in m_nodes
        size_t idx_a;
        size_t idx_b;
    };
}

class BVHAggregate : public Aggregate
{
private:
    std::vector<ObjInfo> m_objInfos;
    std::vector<BVHNode> m_nodes;
    std::vector<const RenderObject*> m_objList;
    size_t m_leafMax = 5;

    /**
     * Build bvh node for m_objInfos in [start, end)
     * @param start start idx in m_objInfos
     * @param end end (not included) idx in m_objInfos
     * @return idx of built-node in m_nodes
     */
    size_t BuildImpl(const size_t start, const size_t end)
    {
        const size_t size = end - start;

        // build leaf node
        if (size <= m_leafMax) {
            BBox box;
            size_t idx_start = m_objList.size();
            for(size_t i = start; i < end; i++) {
                m_objList.push_back(m_objInfos[i].object);
                box |= m_objInfos[i].box;
            }
            size_t idx_end = m_objList.size();

            size_t idx_node = m_nodes.size();
            m_nodes.push_back(BVHNode{std::move(box), true, idx_start, idx_end});
            return idx_node;
        }

        // find the max span axis
        BBox box_root;
        for(size_t i = start; i < end; i++) {
            box_root |= m_objInfos[i].box;
        }

        int split_axis = box_root.MaxSpanAxis();

        // sort the info array
        std::sort(m_objInfos.begin() + start, m_objInfos.begin() + end, [&](const ObjInfo& a, const ObjInfo& b){
            return a.box.m_min[split_axis] + a.box.m_max[split_axis] < b.box.m_min[split_axis] + b.box.m_max[split_axis];
        });

        // build interior node
        const size_t idx_node = m_nodes.size();
        m_nodes.push_back({box_root, false,0, 0});

        const size_t mid = (start + end) / 2;
        m_nodes[idx_node].idx_a = BuildImpl(start, mid);
        m_nodes[idx_node].idx_b = BuildImpl(mid, end);

        return idx_node;
    }

    bool IsIntersectImpl(const Ray& r, const vec3& dir_inv, const BVHNode& node) const
    {
        if (!node.box.IsIntersect(r, dir_inv)) {
            return false;
        }

        // leaf node
        if(node.is_leaf) {
            for (size_t i = node.idx_a; i < node.idx_b; i++) {
                if (m_objList[i]->IsIntersect(r)) {
                    return true;
                }
            }
            return false;
        } else { //interior node
            return IsIntersectImpl(r, dir_inv, m_nodes[node.idx_a]) ||
                   IsIntersectImpl(r, dir_inv, m_nodes[node.idx_b]);
        }
    }

    bool GetIntersectImpl(Ray& r, const vec3& dir_inv, const BVHNode& node, HitPoint *hit_point) const
    {
        if (!node.box.IsIntersect(r, dir_inv)) {
            return false;
        }

        // leaf node
        if(node.is_leaf) {
            bool ret = false;
            for (size_t i = node.idx_a; i < node.idx_b; i++) {
                if (m_objList[i]->GetIntersect(r, hit_point)) {
                    r.t_max = hit_point->t;
                    ret = true;
                }
            }
            return ret;
        } else { //interior node
            return GetIntersectImpl(r, dir_inv, m_nodes[node.idx_a], hit_point) ||
                   GetIntersectImpl(r, dir_inv, m_nodes[node.idx_b], hit_point);
        }
    }

public:
    BVHAggregate(const std::vector<SP<const RenderObject>>& objects) : Aggregate(objects)
    {
        Build();
    }

    void Build()
    {
        if (m_objects.empty()) {
            spdlog::error("Zero object when building bvh!");
            return;
        }

        spdlog::info("Building bvh, total primitive size is {}.", m_objects.size());

        m_objInfos.resize(m_objects.size());
        m_objList.reserve(m_objects.size());

        for(size_t i = 0; i < m_objects.size(); i++) {
            m_objInfos[i] = ObjInfo{m_objects[i].get(), m_objects[i]->WorldBound()};
        }

        // recursively build tree in range [0, end)
        BuildImpl(0, m_objInfos.size());

        spdlog::info("Building BVH done.");
    }

    bool IsIntersect(const Ray &r) const override
    {
        if (m_nodes.size() < 0) {
            return false;
        }

        const vec3 dir_inv(1.0_r/r.dir.x, 1.0_r/r.dir.y, 1.0_r/r.dir.z);
        return IsIntersectImpl(r, dir_inv, m_nodes[0]);
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        if (m_nodes.size() < 0) {
            return false;
        }

        Ray ray = r;
        const vec3 dir_inv(1.0_r/r.dir.x, 1.0_r/r.dir.y, 1.0_r/r.dir.z);
        return GetIntersectImpl(ray, dir_inv, m_nodes[0], hit_point);
    }
};

SP<Aggregate> CreateBVHAggregate(const std::vector<SP<const RenderObject>>& objects)
{
    return MakeSP<BVHAggregate>(objects);
}

FM_ENGINE_END