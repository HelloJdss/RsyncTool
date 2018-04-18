cmake_minimum_required(VERSION 3.1.0)

project(RsyncTool)

include_directories(code/include)
include_directories(code/protocol)
include_directories(code/common/3rd/tinyxml2)
include_directories(code/common/FileUtil)
include_directories(code/common/LogModule)
include_directories(code/common/NetUtil)
include_directories(code/common/Protocol)
include_directories(code/common/RsyncKernel)
include_directories(code/common/ThreadModule)


set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

message(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add reentrant define
add_definitions(-D_REENTRANT)

add_subdirectory(code)
