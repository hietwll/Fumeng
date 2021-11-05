CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(THREAD_POOL)

add_library(thread_pool INTERFACE)

target_include_directories(thread_pool INTERFACE .)