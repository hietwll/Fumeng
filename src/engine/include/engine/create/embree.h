#ifndef FM_ENGINE_CREATE_EMBREE_H
#define FM_ENGINE_CREATE_EMBREE_H

FM_ENGINE_BEGIN

#ifdef USE_EMBREE
void CreateEmbreeDevice();
void DestroyEmbreeDevice();
#endif

FM_ENGINE_END

#endif