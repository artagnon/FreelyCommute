include_directories(include)

add_executable(CLI
  Parser.cpp
  Cli.cpp)

set_target_properties(CLI PROPERTIES
  OUTPUT_NAME "fc-rM"
  CXX_STANDARD 20
  LINKER_LANGUAGE CXX)
