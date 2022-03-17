CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(Fumeng)

#option(USE_FLOAT "Use float precision for calculation" OFF)
#
#if(USE_FLOAT)
#    message("Use float precision for calculation")
#    add_definitions(-DUSE_FLOAT)
#endif(USE_FLOAT)

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     SET(GCC_COVERAGE_COMPILE_FLAGS "-fpermissive")
#     SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
# endif ()

ADD_SUBDIRECTORY(thirdparty/glm)
ADD_SUBDIRECTORY(thirdparty/spdlog)
ADD_SUBDIRECTORY(thirdparty/thread_pool)
ADD_SUBDIRECTORY(thirdparty/tiny_obj_loader)
ADD_SUBDIRECTORY(thirdparty/nlohmann_json)
ADD_SUBDIRECTORY(src/engine)
ADD_SUBDIRECTORY(src/cmd)