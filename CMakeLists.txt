CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(Fumeng)

option(USE_FLOAT "Use single precision (float)." OFF)
option(USE_EMBREE "Use embree for bvh accelerator." OFF)
option(USE_OIDN "Use OIDN for denoising." OFF)

if(USE_FLOAT)
   message("Use single precision (float).")
   add_definitions(-DUSE_FLOAT)
endif(USE_FLOAT)

if(USE_EMBREE)
   message("Use embree for bvh accelerator.")
   add_definitions(-DUSE_EMBREE)
   FIND_PACKAGE(embree 3.0 REQUIRED)
endif(USE_EMBREE)

if(USE_OIDN)
   message("Use OIDN for denoising.")
   add_definitions(-DUSE_OIDN)
   FIND_PACKAGE(OpenImageDenoise REQUIRED)
endif(USE_OIDN)

ADD_SUBDIRECTORY(thirdparty/glm)
ADD_SUBDIRECTORY(thirdparty/spdlog)
ADD_SUBDIRECTORY(thirdparty/thread_pool)
ADD_SUBDIRECTORY(thirdparty/tiny_obj_loader)
ADD_SUBDIRECTORY(thirdparty/nlohmann_json)
ADD_SUBDIRECTORY(src/engine)
ADD_SUBDIRECTORY(src/cmd)