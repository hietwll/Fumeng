#ifndef FUMENG_TRIANGLE_H
#define FUMENG_TRIANGLE_H

#include <engine/core/geometry.h>
#include <engine/core/ray.h>
#include <engine/core/utils.h>
#include <engine/core/hit_point.h>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#ifndef USE_FLOAT
#define TINYOBJLOADER_USE_DOUBLE
#endif

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

    // area
    real m_area;

public:
    Triangle(SP<const TriangleMesh> mesh, const vidx& idx0, const vidx& idx1, const vidx& idx2);
    vec3 GetPos(const vidx& idx) const;
    vec3 GetNormal(const vidx& idx) const;
    vec2 GetUV(const vidx& idx) const;
    bool IsIntersect(const Ray &r) const override;
    bool GetIntersect(const Ray &r, HitPoint *hit_point) const override;
    bool IntersectTriangle(const Ray &r, vec3& bc, real& t) const;
    HitPoint Sample(real *pdf, const vec3 &sample) const override;
    HitPoint Sample(const HitPoint& ref, real *pdf, const vec3 &sample) const override;
    real Pdf(const vec3& shd_pos, const vec3& sample) const override;
    real Area() const override;
    int GetMaxAxis(const vec3& v) const;
    vec3 Permute(const vec3& v, int x, int y, int z) const;
}
;

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

FM_ENGINE_END

#endif