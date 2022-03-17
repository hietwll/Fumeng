CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(Fumeng)

option(USE_FLOAT "Use single precision (float)" OFF)

if(USE_FLOAT)
   message("Use single precision (float)")
   add_definitions(-DUSE_FLOAT)
endif(USE_FLOAT)

ADD_SUBDIRECTORY(thirdparty/glm)
ADD_SUBDIRECTORY(thirdparty/spdlog)
ADD_SUBDIRECTORY(thirdparty/thread_pool)
ADD_SUBDIRECTORY(thirdparty/tiny_obj_loader)
ADD_SUBDIRECTORY(thirdparty/nlohmann_json)
ADD_SUBDIRECTORY(src/engine)
ADD_SUBDIRECTORY(src/cmd)