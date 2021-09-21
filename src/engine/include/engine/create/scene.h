#ifndef FM_ENGINE_CREATE_SCENE_H
#define FM_ENGINE_CREATE_SCENE_H

#include <engine/core/scene.h>

FM_ENGINE_BEGIN

SP<Scene> CreateSimpleScene(SP<const Camera>& camera, SP<const Aggregate>& aggregate);

FM_ENGINE_END

#endif