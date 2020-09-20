cmake_minimum_required(VERSION 3.18.2)
project(fc)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -fdiagnostics-color=always")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(rM)
