cmake_minimum_required(VERSION 3.19.6)
project(fc)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -fdiagnostics-color=always")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(rM)
add_subdirectory(svg)
