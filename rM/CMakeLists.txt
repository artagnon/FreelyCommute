cmake_minimum_required(VERSION 3.18.2)
project(rM)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -fdiagnostics-color=always")
set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(CLI
  "${CMAKE_CURRENT_SOURCE_DIR}/ParseTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Cli.cpp")

set_target_properties(CLI PROPERTIES
  OUTPUT_NAME "fc-rM"
  CXX_STANDARD 20
  LINKER_LANGUAGE CXX)
