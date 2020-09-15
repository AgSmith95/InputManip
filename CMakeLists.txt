cmake_minimum_required(VERSION 3.15)
project(InputManip)

set(CMAKE_CXX_STANDARD 17)

message("it is ${CMAKE_BUILD_TYPE} build")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options(-D_DEBUG)
endif()

add_definitions(-DBUILD_TYPE="${CMAKE_BUILD_TYPE}")

set(
    SOURCE_FILES
    main.cpp
    helpers.cpp helpers.h)
add_executable(InputManip ${SOURCE_FILES})
