cmake_minimum_required(VERSION 3.14)

project(doip-client)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    ./src/**.cpp
    /src/**.h)

add_executable(${CMAKE_PROJECT_NAME} ${sources})
