#ifdef USE_EMBREE

#include <engine/core/aggregate.h>
#include <engine/core/render_object.h>
#include <engine/core/ray.h>
#include <engine/core/embree_device.h>

#include <embree3/rtcore.h>


FM_ENGINE_BEGIN

namespace {
    struct BVHNode;
    struct Leaf;
    struct Interior;

    struct Leaf
    {
        size_t start;
        size_t end;  
    };

    struct Interior
    {
        BVHNode* left;
        BVHNode* right;
        BBox leftBox;
        BBox rightBox;
    };

    struct BVHNode
    {  
        bool is_leaf;
        union 
        {
            Leaf leaf;
            Interior interior;
        };
    };    
}

class EmbreeAggregate : public Aggregate
{
private:
    RTCBVH m_bvh;
    size_t m_leafMax = 5;
    BVHNode* m_root;
    std::vector<const RenderObject*> m_sorted_ptr;

    struct CustomParams
    {
        std::vector<const RenderObject*>& sorted;
        std::vector<SP<const RenderObject>>& raw;
    };

    static void CopyRTCBox(BBox& box, const RTCBounds& bound)
    {
        box.m_min.x = bound.lower_x;
        box.m_min.y = bound.lower_y;
        box.m_min.z = bound.lower_z;

        box.m_max.x = bound.upper_x;
        box.m_max.y = bound.upper_y;
        box.m_max.z = bound.upper_z;        
    }

    //Create interior node
    static void* CreateNode(RTCThreadLocalAllocator alloc, unsigned int num, void* customPtr)
    {
        BVHNode *node = static_cast<BVHNode*>(rtcThreadLocalAlloc(alloc, sizeof(BVHNode), alignof(BVHNode)));
        
        node->is_leaf = false;
        node->interior.left = nullptr;
        node->interior.right = nullptr;

        return node;        
    }

    //Set children for interior node
    static void SetNodeChildren(void* parent, void** child, unsigned int num, void* customPtr)
    {
        BVHNode* node = static_cast<BVHNode*>(parent);
        node->interior.left = static_cast<BVHNode*>(child[0]);
        node->interior.right = static_cast<BVHNode*>(child[1]);
    }

    //Set bounds for interior node
    static void SetNodeBounds(void* parent, const RTCBounds** bounds, unsigned int num, void* customPtr)
    {
        BVHNode* node = static_cast<BVHNode*>(parent);
        CopyRTCBox(node->interior.leftBox, *bounds[0]);
        CopyRTCBox(node->interior.rightBox, *bounds[1]); 
    }        

    //Create leaf node
    static void* CreateLeaf(RTCThreadLocalAllocator alloc, const RTCBuildPrimitive *prim, size_t num, void* customPtr)
    {
        BVHNode *node = static_cast<BVHNode*>(rtcThreadLocalAlloc(alloc, sizeof(BVHNode), alignof(BVHNode)));
        node->is_leaf = true;
        
        CustomParams* customParams = static_cast<CustomParams*>(customPtr);

        const size_t start = customParams->sorted.size();
        for (size_t i = 0; i < num; i++)
        {
            customParams->sorted.push_back(customParams->raw[prim[i].geomID].get());
        }
        const size_t end = customParams->sorted.size();
        
        node->leaf.start = start;
        node->leaf.end = end;

        return node;
    }
    

    bool IsIntersectImpl(const Ray& r, const vec3& dir_inv, const BVHNode& node) const
    {
        if (node.is_leaf) {
            for (size_t i = node.leaf.start; i < node.leaf.end; i++)
            {
                if (m_sorted_ptr[i]->IsIntersect(r)) {
                    return true;
                }
            }
            return false;
        } else { // interior node
            const bool hit_left = node.interior.leftBox.IsIntersect(r, dir_inv);
            if (hit_left) {
                if (IsIntersectImpl(r, dir_inv, *node.interior.left)) {
                    return true;
                }
            }

            const bool hit_right = node.interior.rightBox.IsIntersect(r, dir_inv);
            if (hit_right) {
                if (IsIntersectImpl(r, dir_inv, *node.interior.right)) {
                    return true;
                }
            }
            
            return false;
        }
    }

    bool GetIntersectImpl(Ray& r, const vec3& dir_inv, const BVHNode& node, HitPoint *hit_point) const
    {
        if (node.is_leaf) {
            bool ret = false;
            for (size_t i = node.leaf.start; i < node.leaf.end; i++)
            {
                if (m_sorted_ptr[i]->GetIntersect(r, hit_point)) {
                    r.t_max = hit_point->t;
                    ret = true;
                }
            }
            return ret;
        } else { // interior node
            bool hit_left = node.interior.leftBox.IsIntersect(r, dir_inv);
            if (hit_left) {
                hit_left = GetIntersectImpl(r, dir_inv, *node.interior.left, hit_point);
            }

            bool hit_right = node.interior.rightBox.IsIntersect(r, dir_inv);
            if (hit_right) {
                hit_right = GetIntersectImpl(r, dir_inv, *node.interior.right, hit_point);
            }

            return hit_left || hit_right;
        }
    }    

public:
    explicit EmbreeAggregate(const std::vector<SP<const RenderObject>>& objects) :
    Aggregate(objects),
    m_bvh(nullptr)
    {
        Build();
    }

    ~EmbreeAggregate()
    {
        if(m_bvh) {
            rtcReleaseBVH(m_bvh);
        }
    }

    void Build()
    {
        if (m_objects.empty()) {
            spdlog::warn("No object was found when building bvh!");
            return;
        }

        spdlog::info("Building bvh using embree, total primitive size is {}.", m_objects.size());

        if(m_bvh)
        {
            rtcReleaseBVH(m_bvh);
            m_bvh  = nullptr;
        }

        m_bvh = rtcNewBVH(EbreeDevice::GetDevice());
        std::vector<RTCBuildPrimitive> prims(m_objects.size());

        for (size_t i = 0; i < prims.size(); i++)
        {
            auto &p = prims[i];

            const auto bbox = m_objects[i]->WorldBound();

            p.geomID  = static_cast<unsigned>(i);
            p.primID  = static_cast<unsigned>(i);
            p.lower_x = bbox.m_min.x;
            p.lower_y = bbox.m_min.y;
            p.lower_z = bbox.m_min.z;
            p.upper_x = bbox.m_max.x;
            p.upper_y = bbox.m_max.y;
            p.upper_z = bbox.m_max.z;        
        }

        CustomParams customParams = {m_sorted_ptr, m_objects};

        RTCBuildArguments args = rtcDefaultBuildArguments();
        args.byteSize               = sizeof(args);
        args.buildFlags             = RTC_BUILD_FLAG_NONE;
        args.buildQuality           = RTC_BUILD_QUALITY_HIGH;
        args.maxBranchingFactor     = 2;
        args.maxDepth               = 128;
        args.sahBlockSize           = 1;
        args.minLeafSize            = 1;
        args.maxLeafSize            = m_leafMax;
        args.traversalCost          = 1;
        args.intersectionCost       = 1;
        args.bvh                    = m_bvh;
        args.primitives             = prims.data();
        args.primitiveCount         = prims.size();
        args.primitiveArrayCapacity = prims.capacity();
        args.createNode             = CreateNode;
        args.setNodeChildren        = SetNodeChildren;
        args.setNodeBounds          = SetNodeBounds;
        args.createLeaf             = CreateLeaf;
        args.userPtr                = &customParams;        
        
        m_root = static_cast<BVHNode*>(rtcBuildBVH(&args));
    }

    bool IsIntersect(const Ray &r) const override
    {
        if (m_objects.size() == 0) {
            return false;
        }

        const vec3 dir_inv(1.0_r/r.dir.x, 1.0_r/r.dir.y, 1.0_r/r.dir.z);
        return IsIntersectImpl(r, dir_inv, *m_root);
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        if (m_objects.size() == 0) {
            return false;
        }

        Ray ray = r;
        const vec3 dir_inv(1.0_r/r.dir.x, 1.0_r/r.dir.y, 1.0_r/r.dir.z);
        return GetIntersectImpl(ray, dir_inv, *m_root, hit_point);
    }    
};

SP<Aggregate> CreateEmbreeAggregate(const std::vector<SP<const RenderObject>>& objects)
{
    return MakeSP<EmbreeAggregate>(objects);
}

FM_ENGINE_END

#endif