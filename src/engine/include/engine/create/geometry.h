#ifndef FM_ENGINE_CREATE_GEOMETRY_H
#define FM_ENGINE_CREATE_GEOMETRY_H

#include <engine/core/geometry.h>

FM_ENGINE_BEGIN

SP<Geometry> CreateSphere(real radius, const Transform& to_world);
std::vector<SP<const Geometry>> CreateTriangleMesh(const std::string& filename);

FM_ENGINE_END

#endif