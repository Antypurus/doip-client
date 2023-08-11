cmake_minimum_required(VERSION 3.14)

project(doip-client)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    ./src/**.cpp
    /src/**.h)

add_executable(${CMAKE_PROJECT_NAME} ${sources})
