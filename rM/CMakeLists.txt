find_package(CURL 7.64.1 REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable("fc-rM"
  File.cpp)

add_executable("fc-rM-cloud"
  Cloud.cpp)

target_link_libraries("fc-rM-cloud" ${CURL_LIBRARIES})

add_executable("fc-rM-test"
  Test.cpp)

add_test(NAME ".rM v5 testsuite"
  COMMAND "fc-rM-test"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
