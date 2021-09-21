#ifndef FM_ENGINE_CREATE_AGGREGATE_H
#define FM_ENGINE_CREATE_AGGREGATE_H

#include <engine/core/aggregate.h>

FM_ENGINE_BEGIN

SP<Aggregate> CreateSimpleAggregate(const std::vector<SP<const RenderObject>>& objects);

FM_ENGINE_END

#endif