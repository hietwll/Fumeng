#ifndef FUMENG_TRIANGLE_H
#define FUMENG_TRIANGLE_H

#include <engine/core/geometry.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>
#include <string>

#include <tiny_obj_loader.h>

FM_ENGINE_BEGIN

class TriangleMesh;

class Triangle : public Geometry
{
private:
    using vidx = tinyobj::index_t;
    SP<const TriangleMesh> m_mesh;
    const vidx& m_idx0;
    const vidx& m_idx1;
    const vidx& m_idx2;
    vec3 m_v0, m_v1, m_v2;

    // area
    real m_area;

public:
    Triangle(SP<const TriangleMesh> mesh, const vidx& idx0, const vidx& idx1, const vidx& idx2);
    vec3 GetPos(const vidx& idx) const;
    vec3 GetNormal(const vidx& idx) const;
    vec2 GetUV(const vidx& idx) const;
    bool IsIntersect(const Ray &r) const override;
    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override;
    HitPoint Sample(real *pdf, const vec3 &sample) const override;
    HitPoint Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const override;
    real Pdf(const vec3& shd_pos, const vec3& sample) const override;
    real Area() const override;
    BBox WorldBound() const override;
};

class TriangleMesh
{
public:
    explicit TriangleMesh(const std::string& inputfile);
    bool m_hasNormal = true;
    bool m_hasUV = true;

    // obj file
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
};

namespace triangle_func {
    bool IntersectTriangle(const Ray &r, vec3 v0, vec3 v1, vec3 v2,
                           vec3& bc, real& t);
    int GetMaxAxis(const vec3& v);
    vec3 Permute(const vec3& v, int x, int y, int z);                      
}

FM_ENGINE_END

#endif