cmake_minimum_required(VERSION 3.5)

project(mm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -Wall")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(mmtest test/mmtest.cpp)
add_executable(buffertest test/buffertest.cpp)

target_include_directories(mmtest PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(buffertest PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(mmtest ${GTEST_BOTH_LIBRARIES})
target_link_libraries(buffertest ${GTEST_BOTH_LIBRARIES})

enable_testing()
add_test(NAME MMTest COMMAND mmtest)
add_test(NAME BufferTest COMMAND buffertest)
