
cmake_minimum_required(VERSION 3.8)
project(ParallelShared)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c fromServer.cpp fromServer.h)
add_executable(ParallelShared ${SOURCE_FILES})
target_link_libraries(ParallelShared pthread;m)