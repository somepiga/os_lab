cmake_minimum_required(VERSION 3.5)

project(mm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14 -Wall")

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(mmtest test/mmtest.cpp)
add_executable(buffertest test/buffertest.cpp)
target_link_libraries(mmtest ${GTEST_BOTH_LIBRARIES})
target_link_libraries(buffertest ${GTEST_BOTH_LIBRARIES})

add_test(NAME MMTest COMMAND mmtest)
add_test(NAME BufferTest COMMAND buffertest)
enable_testing()
