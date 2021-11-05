CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(Fumeng)

#option(USE_FLOAT "Use float precision for calculation" OFF)
#
#if(USE_FLOAT)
#    message("Use float precision for calculation")
#    add_definitions(-DUSE_FLOAT)
#endif(USE_FLOAT)

ADD_SUBDIRECTORY(thirdparty/glm)
ADD_SUBDIRECTORY(thirdparty/spdlog)
ADD_SUBDIRECTORY(thirdparty/thread_pool)
ADD_SUBDIRECTORY(src/engine)
ADD_SUBDIRECTORY(src/cmd)