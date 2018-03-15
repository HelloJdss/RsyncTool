#添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/code/common/)
include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)

#添加可执行文件
add_executable(selectserver selectserver.cpp)

#添加静态链接库
target_link_libraries(selectserver LogModule)