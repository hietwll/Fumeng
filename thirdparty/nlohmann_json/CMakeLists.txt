CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(NLOHMANN_JSON)

add_library(nlohmann_json INTERFACE)

target_include_directories(nlohmann_json INTERFACE .)