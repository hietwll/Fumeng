CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(Fumeng)

option(USE_FLOAT "Use float precision for calculation" ON)

if(USE_FLOAT)
    message("USE_FLOAT")
    add_definitions(-DUSE_FLOAT)
endif(USE_FLOAT)

ADD_SUBDIRECTORY(thirdparty/glm)
ADD_SUBDIRECTORY(src/engine)
ADD_SUBDIRECTORY(src/cmd)