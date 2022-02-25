#include "triangle.h"

FM_ENGINE_BEGIN

TriangleMesh::TriangleMesh(const std::string& inputfile)
{
    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

    if (!ret)
    {
        spdlog::error("Loading model failed: {}.", inputfile);
        return;
    }
}


Triangle::Triangle(SP<const TriangleMesh> mesh, const vidx& idx0, const vidx& idx1, const vidx& idx2)
: Geometry(Transform()),
  m_mesh(mesh),
  m_idx0(idx0),
  m_idx1(idx1),
  m_idx2(idx2)
{
    const auto e0 = GetPos(m_idx1) - GetPos(m_idx0);
    const auto e1 = GetPos(m_idx2) - GetPos(m_idx0);
    const auto pg = cross(e0 , e1);
    m_area = glm::length(pg) * 0.5_r;
}
    
vec3 Triangle::GetPos(const vidx& idx) const
{
    size_t bg = 3 * idx.vertex_index;
    return {m_mesh->attrib.vertices[bg], m_mesh->attrib.vertices[bg + 1], m_mesh->attrib.vertices[bg + 2]};
}
    
vec3 Triangle::GetNormal(const vidx& idx) const
{
    size_t bg = 3 * idx.normal_index;
    return {m_mesh->attrib.normals[bg], m_mesh->attrib.normals[bg + 1], m_mesh->attrib.normals[bg + 2]};
}
    
vec2 Triangle::GetUV(const vidx& idx) const
{
    if (idx.texcoord_index >= 0) {
        size_t bg = 2 * idx.texcoord_index;
        return {m_mesh->attrib.texcoords[bg], 1.0_r - m_mesh->attrib.texcoords[bg + 1]};
    } else {
        return {0.0_r, 0.0_r};
    }
}

bool Triangle::IsIntersect(const Ray &r) const
{
    vec3 bc;
    real t;
    return IntersectTriangle(r, bc, t);
}

bool Triangle::GetIntersect(const Ray &r, HitPoint *hit_point) const
{
    vec3 bc;
    real t;
    bool intersected = IntersectTriangle(r, bc, t);

    if (!intersected) {
        return false;
    }

    if (t > hit_point->t || t <= 0.0_r) {
        return false;
    }

    hit_point->t = t;
    hit_point->pos = r(t);
    hit_point->wo_r_w = -r.dir;

    const vec2 uv = bc.x * GetUV(m_idx0) +
                    bc.y * GetUV(m_idx1) +
                    bc.z * GetUV(m_idx2);

    hit_point->uv.x = uv.x;
    hit_point->uv.y = uv.y;

    if (m_idx0.normal_index >= 0 &&
        m_idx1.normal_index >= 0 &&
        m_idx2.normal_index >= 0) {
        hit_point->ng = bc.x * GetNormal(m_idx0) +
                        bc.y * GetNormal(m_idx1) +
                        bc.z * GetNormal(m_idx2);
    } else {
        hit_point->ng = glm::normalize(glm::cross(GetPos(m_idx0) - GetPos(m_idx2),
                                                  GetPos(m_idx1) - GetPos(m_idx2)));
    }
    hit_point->ns = hit_point->ng;

    hit_point->ss = glm::normalize(GetPos(m_idx1) - GetPos(m_idx0));

    return true;
}

bool Triangle::IntersectTriangle(const Ray &r, vec3& bc, real& t) const
{
    // transform vertex position to ray space
    vec3 v0 = GetPos(m_idx0);
    vec3 v1 = GetPos(m_idx1);
    vec3 v2 = GetPos(m_idx2);

    // translate
    v0 -= r.ori;
    v1 -= r.ori;
    v2 -= r.ori;

    // get main axis
    int az = GetMaxAxis(r.dir);
    int ax = (az + 1) % 3;
    int ay = (ax + 1) % 3;

    // pick max axis
    vec3 dir = Permute(r.dir, ax, ay, az);
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
    t = (e0 * v0.z + e1 * v1.z + e2 * v2.z) * invDet;

    if (t <= r.t_min || t >= r.t_max) {
        return false;
    }

    // get barycentric coordinates
    bc.x = e0 * invDet;
    bc.y = e1 * invDet;
    bc.z = e2 * invDet;

    return true;
}

HitPoint Triangle::Sample(real *pdf, const vec3 &sample) const
{
    // get uniformed distributed uv
    const real sqrt_u = std::sqrt(sample.x);
    const real b0 = 1.0_r - sqrt_u;
    const real b1 = sample.y * sqrt_u;
    const real b2 = 1.0_r - b0 - b1;

    HitPoint hitPoint;
    hitPoint.pos = b0 * GetPos(m_idx0) + b1 * GetPos(m_idx1) + b2 * GetPos(m_idx2);

    const vec2 uv = b0 * GetUV(m_idx0) +
                    b1 * GetUV(m_idx1) +
                    b2 * GetUV(m_idx2);

    hitPoint.uv.x = uv.x;
    hitPoint.uv.y = uv.y;

    if (m_idx0.normal_index >= 0 &&
        m_idx1.normal_index >= 0 &&
        m_idx2.normal_index >= 0) {
        hitPoint.ng = b0 * GetNormal(m_idx0) +
                      b1 * GetNormal(m_idx1) +
                      b2 * GetNormal(m_idx2);
    } else {
        hitPoint.ng = glm::normalize(glm::cross(GetPos(m_idx0) - GetPos(m_idx2), GetPos(m_idx1) - GetPos(m_idx2)));
    }

    hitPoint.ns = hitPoint.ng;

    hitPoint.ss = glm::normalize(GetPos(m_idx1) - GetPos(m_idx0));

    *pdf = Pdf(hitPoint.pos, sample);

    return hitPoint;
}

HitPoint Triangle::Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const
{
    return Sample(pdf, sample);
}

real Triangle::Pdf(const vec3& shd_pos, const vec3& sample) const
{
    return 1.0 / m_area;
}

real Triangle::Area() const
{
    return m_area;
}

int Triangle::GetMaxAxis(const vec3& v) const
{
    if (abs(v[0]) > abs(v[1]) && abs(v[0]) > abs(v[2]))
        return 0;
    return abs(v[1]) > abs(v[2]) ? 1 : 2;
}

vec3 Triangle::Permute(const vec3& v, int x, int y, int z) const
{
    return vec3(v[x], v[y], v[z]);
}

BBox Triangle::WorldBound() const
{
    BBox box;

    vec3 v0 = GetPos(m_idx0);
    vec3 v1 = GetPos(m_idx1);
    vec3 v2 = GetPos(m_idx2);

    box |= v0;
    box |= v1;
    box |= v2;

    return box;
}

std::vector<SP<const Geometry>> CreateTriangleMesh(const std::string& filename)
{
    auto mesh = MakeSP<TriangleMesh>(filename);

    std::vector<SP<const Geometry>> triangles;

    // Loop over shapes
    for (size_t s = 0; s < mesh->shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < mesh->shapes[s].mesh.num_face_vertices.size(); f++) {
            triangles.push_back(MakeSP<Triangle>(mesh,
                                           mesh->shapes[s].mesh.indices[index_offset + 0],
                                           mesh->shapes[s].mesh.indices[index_offset + 1],
                                           mesh->shapes[s].mesh.indices[index_offset + 2]));
            size_t fv = size_t(mesh->shapes[s].mesh.num_face_vertices[f]);
            index_offset += fv;
        }
    }

    return triangles;
}

FM_ENGINE_END
