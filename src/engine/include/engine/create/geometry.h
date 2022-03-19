#ifndef FM_ENGINE_CREATE_GEOMETRY_H
#define FM_ENGINE_CREATE_GEOMETRY_H

#include <engine/core/geometry.h>

FM_ENGINE_BEGIN

SP<Geometry> CreateSphere(const SphereConfig& config);
SP<Geometry> CreateSphere(real radius, const Transform& to_world);
void CreateTriangleMesh(const std::string& filename, std::vector<SP<const Geometry>>& geometries);
SP<Geometry> CreateRectangle(const RectangleConfig& config);
SP<Geometry> CreateEmbreeTriangle(const std::string& filename);

FM_ENGINE_END

#endif