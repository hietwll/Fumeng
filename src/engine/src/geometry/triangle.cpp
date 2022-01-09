#include <engine/core/geometry.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>

FM_ENGINE_BEGIN

class TriangleMesh
{
public:
    // TriangleMesh Public Methods
    TriangleMesh(const Transform &ObjectToWorld, int nTriangles,
                 const int *vertexIndices, int nVertices, const Point3f *P,
                 const Vector3f *S, const Normal3f *N, const Point2f *uv,
                 const std::shared_ptr<Texture<Float>> &alphaMask,
                 const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                 const int *faceIndices);

    const size_t m_nTriangles
    const size_t m_nVertices;
    bool m_hasNormal;
    bool m_hasTangent;
    std::vector<size_t> m_vertexBuffer;
    std::vector<vec3> m_position;
    std::vector<vec3> m_normal;
    std::vector<vec3> m_tangent;
    std::vector<vec2> m_uv;
};

struct MeshIdx
{
    size_t m_idx[3] = {-1};
};

class Triangle : public Geometry
{
private:
    const TriangleMesh* m_triangleMesh;
    const MeshIdx& m_meshIdx;

public:
    Triangle(const TriangleMesh* triangleMesh, const MeshIdx& meshIdx)
    : m_triangleMesh(triangleMesh), m_meshIdx(meshIdx)
    {
    }

    bool IsIntersect(const Ray &r) const override
    {
    }

    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override
    {
        const auto idx0 = m_meshIdx.m_idx[0];
        const auto idx1 = m_meshIdx.m_idx[1];
        const auto idx2 = m_meshIdx.m_idx[2];

        // vertex position in world space
        const auto& v0_w = m_triangleMesh->m_position[idx0];
        const auto& v1_w = m_triangleMesh->m_position[idx1];
        const auto& v2_w = m_triangleMesh->m_position[idx2];

        // uv coordinate
        const vec2& uv0 = m_triangleMesh->m_uv[idx0];
        const vec2& uv1 = m_triangleMesh->m_uv[idx1];
        const vec2& uv2 = m_triangleMesh->m_uv[idx2];

        // transform vertex position to ray space
        vec3 v0 = v0_w;
        vec3 v1 = v1_w;
        vec3 v2 = v2_w;

        // translate
        v0 -= r.ori;
        v1 -= r.ori;
        v2 -= r.ori;

        // get main axis
        size_t az = GetMaxAxis(r.dir);
        size_t ax = (az + 1) % 3;
        size_t ay = (ax + 1) % 3;

        // pick max axis
        vec3 dir = Permute(r.ori, ax, ay, az);
        v0 = Permute(v0, ax, ay, az);
        v1 = Permute(v1, ax, ay, az);
        v2 = Permute(v2, ax, ay, az);

        // shear
        real sx = -dir.x / dir.z;
        real sy = -dir.y / dir.z;
        real sz = 1.0_r / dir.z;

        v0.x += sx * v0.z;
        v0.y += sy * v0.z;
        v1.x += sx * v1.z;
        v1.y += sy * v1.z;
        v2.x += sx * v2.z;
        v2.y += sy * v2.z;

        // compute edge function coefficients
        real e0 = v1.x * v2.y - v1.y * v2.x;
        real e1 = v2.x * v0.y - v2.y * v0.x;
        real e2 = v0.x * v1.y - v0.y * v1.x;

        // fall back to double precision
        if(sizeof(real) == sizeof(float) &&
            (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)){
            e0 = (real)((double)v1.x * (double)v2.y - (double)v1.y * (double)v2.x);
            e1 = (real)((double)v2.x * (double)v0.y - (double)v2.y * (double)v0.x);
            e2 = (real)((double)v0.x * (double)v1.y - (double)v0.y * (double)v1.x);
        }

        if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
            return false;
        }

        const real det = e0 + e1 + e2;
        if (det == 0) {
            return false;
        }

        v0.z *= sz;
        v1.z *= sz;
        v2.z *= sz;

        const real invDet = 1.0_r / det;
        const real t = (e0 * p0.z + e1 * p1.z + e2 * p2.z) * invDet;

        if (t <= r.t_min || t >= r.t_max) {
            return false;
        }

        if (t > hit_point->t || t <= 0.0_r) {
            return false;
        }

        // get barycentric coordinates
        const real b0 = e0 * invDet;
        const real b1 = e1 * invDet;
        const real b2 = e2 * invDet;

        hit_point->t = t;
        hit_point->pos = r(t);
        hit_point->wo_r_w = -r.dir;

        const vec2 uv = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];
        hit_point->uv.x = uv.x;
        hit_point->uv.y = uv.y;

        if (m_triangleMesh->m_hasNormal) {
            hit_point->ng = b0 * m_triangleMesh->m_normal[idx0] +
                            b1 * m_triangleMesh->m_normal[idx1] +
                            b2 * m_triangleMesh->m_normal[idx2];
        } else {
            hit_point->ng = glm::normalize(glm::cross(v0_w - v2_w, v1_w - v2_w));
        }
        hit_point->ns = hit_point->ng;

        if (m_triangleMesh->m_hasTangent) {
            hit_point->ss = b0 * m_triangleMesh->m_tangent[idx0] +
                    b1 * m_triangleMesh->m_tangent[idx1] +
                    b2 * m_triangleMesh->m_tangent[idx2];
        } else {
            hit_point->ss = GetTangetFromDpDu();
        }
    }

    size_t GetMaxAxis(const vec3& v)
    {
        if (abs(v[0]) > abs(v[1]) && abs(v[0]) > abs(v[2]))
            return 0;
        return abs(v[1]) > abs(v[2]) ? 1 : 2;
    }

    vec3 Permute(const vec3& v, size_t x, size_t y, size_t z) {
        return vec3(v[x], v[y], v[z]);
    }
}


FM_ENGINE_END
