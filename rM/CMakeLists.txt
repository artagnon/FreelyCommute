add_executable("fc-rM"
  File.cpp)

add_executable("fc-rM-test"
  Test.cpp)

add_test(NAME ".rM v5 testsuite"
  COMMAND "fc-rM-test"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
