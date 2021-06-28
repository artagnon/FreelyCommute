add_executable("fc-server"
  Server.cpp)

target_link_libraries("fc-server" "{CMAKE_CURRENT_SOURCE_DIR}/liboatpp.a")

add_test(NAME "oatpp server testsuite"
  COMMAND "fc-server-test"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
