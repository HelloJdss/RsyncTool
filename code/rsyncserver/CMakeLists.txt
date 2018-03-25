include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/Protocol)
include_directories(${PROJECT_SOURCE_DIR}/code/common/NetUtil)
include_directories(${PROJECT_SOURCE_DIR}/code/common/RsyncKernel)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd/flatbuffers)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd)
include_directories(${PROJECT_SOURCE_DIR}/code/common/)


set(SRC_LIST main.cpp MainMod.cpp NetMod.cpp)

#添加可执行文件
add_executable(rsyncserver ${SRC_LIST})

#添加链接库
target_link_libraries(rsyncserver LogModule pthread NetUtil)