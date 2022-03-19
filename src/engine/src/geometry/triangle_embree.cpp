#ifdef USE_EMBREE

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>

#include "triangle.h"
#include <engine/core/embree_device.h>
#include <engine/core/distribution.h>

FM_ENGINE_BEGIN

class TriangleEmbree : public Geometry
{
private:
    using vidx = tinyobj::index_t;
    struct Vertex
    {
        real x, y, z, w;
    };

    struct Index
    {
        uint32_t v0, v1, v2;
    };

    real m_area_sum;
    SP<TriangleMesh> m_mesh;
    RTCScene m_embree_scene = nullptr;
    unsigned int m_embree_geom_id = 0;
    std::vector<Triangle> m_triangles;
    std::vector<real> m_areas;
    UP<Distribution1D> m_1d_dist;
    BBox m_box_world;

    void CheckEmbreeError();

public:
    TriangleEmbree(const std::string& filename);
    bool IsIntersect(const Ray &r) const override;
    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override;
    HitPoint Sample(real *pdf, const vec3 &sample) const override;
    HitPoint Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const override;
    real Pdf(const vec3& shd_pos, const vec3& sample) const override;
    real Area() const override;
    BBox WorldBound() const override;
};

TriangleEmbree::TriangleEmbree(const std::string& filename)
{
    m_mesh = MakeSP<TriangleMesh>(filename);

    // Loop over shapes
    for (size_t s = 0; s < m_mesh->shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < m_mesh->shapes[s].mesh.num_face_vertices.size(); f++) {
            m_triangles.push_back({m_mesh,
                                   m_mesh->shapes[s].mesh.indices[index_offset + 0],
                                   m_mesh->shapes[s].mesh.indices[index_offset + 1],
                                   m_mesh->shapes[s].mesh.indices[index_offset + 2]});
            size_t fv = size_t(m_mesh->shapes[s].mesh.num_face_vertices[f]);
            index_offset += fv;
        }
    }

    // build embree scene
    RTCDevice embree_device = EbreeDevice::GetDevice();
    m_embree_scene = rtcNewScene(embree_device);
    if (!m_embree_scene) {
        CheckEmbreeError();
    }

    // create embree geometry
    RTCGeometry embree_geom = rtcNewGeometry(embree_device, RTC_GEOMETRY_TYPE_TRIANGLE);
    if (!embree_geom) {
        CheckEmbreeError();
    }

    // create geometry vertices buffer
    const size_t triangle_count = m_triangles.size();
    const size_t vertex_count = triangle_count * 3;
    auto vertices = static_cast<Vertex*>(rtcSetNewGeometryBuffer(
                    embree_geom, 
                    RTC_BUFFER_TYPE_VERTEX,
                    0,
                    RTC_FORMAT_FLOAT3,                                 
                    sizeof(Vertex),
                    vertex_count));

    // create geometry index buffer
    auto indices = static_cast<Index*>(rtcSetNewGeometryBuffer(embree_geom, 
                    RTC_BUFFER_TYPE_INDEX,
                    0,
                    RTC_FORMAT_UINT3,                                 
                    sizeof(Index),
                    triangle_count));

    auto copy_vertices = [](Vertex& v, const vec3& pos) {
        v.x = pos.x;
        v.y = pos.y;
        v.z = pos.z;
        v.w = 1.0f; // for padding
    };

    //fill the buffer
    for (size_t i = 0; i < triangle_count; i++)
    {
        const auto& triangle = m_triangles[i];
        copy_vertices(vertices[i + 0], triangle.GetVertex(0));
        copy_vertices(vertices[i + 1], triangle.GetVertex(1));
        copy_vertices(vertices[i + 2], triangle.GetVertex(2));

        const size_t idx = i * 3;
        indices[i].v0 = static_cast<uint32_t>(idx + 0);
        indices[i].v1 = static_cast<uint32_t>(idx + 1);
        indices[i].v2 = static_cast<uint32_t>(idx + 2);        
    }

    // get area, 1d sampler and bounding box
    m_areas.reserve(triangle_count);
    m_area_sum = 0.0_r;
    for(const auto& tri : m_triangles) {
        m_area_sum += tri.Area();
        m_areas.push_back(tri.Area());
        m_box_world |= tri.GetVertex(0);
        m_box_world |= tri.GetVertex(1);
        m_box_world |= tri.GetVertex(2);
    }

    m_1d_dist = MakeUP<Distribution1D>(MakeSP<const std::vector<real>>(m_areas));

    // build geometry and scene
    rtcSetGeometryBuildQuality(embree_geom, RTC_BUILD_QUALITY_HIGH);
    rtcCommitGeometry(embree_geom);

    m_embree_geom_id = rtcAttachGeometry(m_embree_scene, embree_geom);
    
    rtcSetSceneBuildQuality(m_embree_scene, RTC_BUILD_QUALITY_HIGH);
    rtcCommitScene(m_embree_scene);    
        
}

void TriangleEmbree::CheckEmbreeError()
{
    const RTCError err = rtcGetDeviceError(EbreeDevice::GetDevice());
    std::string err_msg;

    switch (err)
    {
    case RTC_ERROR_NONE:
        err_msg = "embree no error";
        break;
    case RTC_ERROR_UNKNOWN:
        err_msg = "embree unknown error";
        break;
    case RTC_ERROR_INVALID_ARGUMENT:
        err_msg = "embree invalid argument";
        break;
    case RTC_ERROR_INVALID_OPERATION:
        err_msg = "embree invalid operation";
        break;
    case RTC_ERROR_OUT_OF_MEMORY:
        err_msg = "embree out of memory";
        break;
    case RTC_ERROR_UNSUPPORTED_CPU:
        err_msg = "embree unsuppoted cpu";
        break;
    case RTC_ERROR_CANCELLED:
        err_msg = "embree got canceled";
        break;                                        
    default:
        err_msg = "embree unknow error code";
        break;
    }

    spdlog::error(err_msg);

    throw std::runtime_error(err_msg);    
}

bool TriangleEmbree::IsIntersect(const Ray &r) const
{
    RTCRay ray = {
        r.ori.x, r.ori.y, r.ori.z,
        r.t_min,
        r.dir.x, r.dir.y, r.dir.z,
        0,
        r.t_max,
        static_cast<unsigned>(-1), 0, 0
    };

    RTCIntersectContext inct_ctx{};
    rtcInitIntersectContext(&inct_ctx);
    rtcOccluded1(m_embree_scene, &inct_ctx, &ray);
    return ray.tfar < 0 && std::isinf(ray.tfar);
}

bool TriangleEmbree::GetIntersect(const Ray &r, HitPoint *hit_point) const
{
    alignas(16) RTCRayHit rayhit = {
    {
        r.ori.x, r.ori.y, r.ori.z,
        r.t_min,
        r.dir.x, r.dir.y, r.dir.z,
        0,
        r.t_max,
        static_cast<unsigned>(-1), 0, 0
    }, { } };

    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;

    RTCIntersectContext inct_ctx{};
    rtcInitIntersectContext(&inct_ctx);
    rtcIntersect1(m_embree_scene, &inct_ctx, &rayhit);
    if(rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
        return false;

    real t = rayhit.ray.tfar;
    hit_point->t = t;
    hit_point->pos = r(t);
    hit_point->wo_r_w = -r.dir;

    const auto& triangle = m_triangles[rayhit.hit.primID];

    //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle
    const real u = rayhit.hit.u;
    const real v = rayhit.hit.v;
    const real w = 1.0_r - u - v;

    const vec2 uv = w * triangle.GetUV(0) +
                    u * triangle.GetUV(1) +
                    v * triangle.GetUV(2);

    hit_point->uv.x = uv.x;
    hit_point->uv.y = uv.y;

    if (triangle.HasNormal()) {
        hit_point->ng = w * triangle.GetNormal(0) +
                        u * triangle.GetNormal(1) +
                        v * triangle.GetNormal(2);
    } else {
        hit_point->ng = glm::normalize(glm::cross(triangle.GetVertex(0) - triangle.GetVertex(2),
                                                  triangle.GetVertex(1) - triangle.GetVertex(2)));
    }
    hit_point->ns = hit_point->ng;

    hit_point->ss = glm::normalize(triangle.GetVertex(1) - triangle.GetVertex(0));

    return true;    
}

HitPoint TriangleEmbree::Sample(real *pdf, const vec3 &sample) const
{
    const auto& sample_1d_info = m_1d_dist->Sample(sample.z);
    assert(sample_1d_info.idx >= 0 && sample_1d_info.idx < m_triangles.size());
    *pdf = 1.0_r / m_area_sum;
    real pdf_tmp;
    return m_triangles[sample_1d_info.idx].Sample(&pdf_tmp, sample);
}

HitPoint TriangleEmbree::Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const
{
    return Sample(pdf, sample);
}

real TriangleEmbree::Pdf(const vec3& shd_pos, const vec3& sample) const
{
    return 1.0_r / Area();
}

real TriangleEmbree::Area() const
{
    return m_area_sum;
}

BBox TriangleEmbree::WorldBound() const
{
    return m_box_world;
}

FM_ENGINE_END

#endif