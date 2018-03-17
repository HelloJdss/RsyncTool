cmake_minimum_required(VERSION 3.09)

project(RsyncTool)

include_directories(code code/common)
set(CMAKE_CXX_STANDARD 11)

message(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add reentrant define
add_definitions(-D_REENTRANT)

add_subdirectory(code)