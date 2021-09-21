#ifndef FM_ENGINE_CREATE_GEOMETRY_H
#define FM_ENGINE_CREATE_GEOMETRY_H

#include <engine/core/geometry.h>

FM_ENGINE_BEGIN

SP<Geometry> CreateSphere(real radius, const Transform& to_world);

FM_ENGINE_END

#endif